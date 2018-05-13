#include <ctype.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/file.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/iter.h"
#include "cixl/lib.h"
#include "cixl/lib/rec.h"
#include "cixl/op.h"
#include "cixl/pair.h"
#include "cixl/parse.h"
#include "cixl/rec.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

static ssize_t rec_eval(struct cx_rmacro_eval *eval,
			struct cx_bin *bin,
			size_t tok_idx,
			struct cx *cx) {
  struct cx_tok *t = cx_vec_get(&eval->toks, 0);
  
  cx_op_new(bin,
	    CX_OTYPEDEF(),
	    tok_idx)->as_typedef.type = t->as_ptr;

  return tok_idx+1;
}

static bool rec_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  int row = cx->row, col = cx->col;
  bool ok = false;
  
  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, cx->row, cx->col, "Missing rec id");
    goto exit1;
  }

  struct cx_tok id = *(struct cx_tok *)cx_vec_pop(&toks);

  if (id.type != CX_TID()) {
    cx_error(cx, id.row, id.col, "Invalid rec id: %s", id.type->id);
    goto exit2;
  }

  char *s = id.as_ptr;

  if (!isupper(s[0])) {
    cx_error(cx, id.row, id.col, "Invalid rec id: %s", s);
    goto exit2;
  }

  struct cx_type *type = cx_get_type(cx, s, true);
  
  if (type && !cx_is(type, cx->rec_type)) {
      cx_error(cx, id.row, id.col, "Attempt to redefine %s as rec", type->id);
      goto exit2;
  }
  
  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, cx->row, cx->col, "Missing rec parents");
    goto exit2;
  }
  
  struct cx_vec parents;
  cx_vec_init(&parents, sizeof(struct cx_type *));
  struct cx_tok parents_tok = *(struct cx_tok *)cx_vec_peek(&toks, 0);

  if (parents_tok.type == CX_TGROUP()) {
    cx_do_vec(&parents_tok.as_vec, struct cx_tok, t) {
      if (t->type != CX_TID()) {
	cx_error(cx, t->row, t->col, "Invalid rec parent");
	goto exit3;
      }
      
      struct cx_type *pt = cx_get_type(cx, t->as_ptr, false);
      
      if (pt->meta != CX_TYPE_ID && pt->meta != CX_TYPE_REC) {
	cx_error(cx, t->row, t->col, "Invalid rec parent: %s", pt->id);
	goto exit3;
      }

      *(struct cx_type **)cx_vec_push(&parents) = pt;
    }

    cx_vec_pop(&toks);
    cx_tok_deinit(&parents_tok);
  }
  
  if (!cx_parse_end(cx, in, &toks, false)) {
    if (!cx->errors.count) { cx_error(cx, cx->row, cx->col, "Missing rec end"); }
    goto exit3;
  }

  struct cx_rec_type *rec_type = type
    ? cx_baseof(type, struct cx_rec_type, imp)
    : cx_test(cx_add_rec_type(*cx->lib, id.as_ptr));
  
  if (type) { cx_rec_type_reinit(rec_type); }

  struct cx_vec fids, ftypes;
  cx_vec_init(&fids, sizeof(struct cx_tok));
  cx_vec_init(&ftypes, sizeof(struct cx_type *));

  bool push_type(struct cx_type *t, int row, int col) {
    if (fids.count == ftypes.count) {
      cx_error(cx, row, col, "Missing field for type: %s", t->id);
      return false;
    }

    for (size_t i = ftypes.count; i < fids.count; i++) {
      *(struct cx_type **)cx_vec_push(&ftypes) = t;
    }

    return true;
  }
  
  cx_do_vec(&toks, struct cx_tok, t) {
    if (t->type == CX_TID()) {
      char *s = t->as_ptr;
      
      if (isupper(s[0])) {
	struct cx_type *ft = cx_get_type(cx, s, false);
	if (!ft || !push_type(ft, t->row, t->col)) { goto exit4; }
      } else {
	*(struct cx_tok *)cx_vec_push(&fids) = *t;
      }
    } else {
      cx_error(cx, t->row, t->col, "Invalid rec tok: %s", t->type->id);
      goto exit4;
    }
  }

  if (fids.count > ftypes.count) { push_type(cx->any_type, -1, -1); }

  for (size_t i=0; i < fids.count; i++) {
    struct cx_tok *fid = cx_vec_get(&fids, i);
    struct cx_type **type = cx_vec_get(&ftypes, i);

    if (!cx_add_field(rec_type, cx_sym(cx, fid->as_ptr), *type, false)) {
      goto exit4;
    }
  }

  cx_do_vec(&parents, struct cx_type *, pt) { cx_derive_rec(rec_type, *pt); }
  struct cx_rmacro_eval *eval = cx_rmacro_eval_new(rec_eval);
  cx_tok_init(cx_vec_push(&eval->toks), CX_TTYPE(), row, col)->as_ptr = rec_type;
  cx_tok_init(cx_vec_push(out), CX_TRMACRO(), row, col)->as_ptr = eval;
  ok = true;
 exit4:
  cx_vec_deinit(&fids);	      
  cx_vec_deinit(&ftypes);	      
 exit3:
  cx_vec_deinit(&parents);
 exit2:
  cx_tok_deinit(&id);
 exit1: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

static bool get_imp(struct cx_call *call) {
  struct cx_sym *f = &cx_test(cx_call_arg(call, 1))->as_sym;
  struct cx_box *r = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  struct cx_rec_type *rt = cx_baseof(r->type, struct cx_rec_type, imp);
  
  if (!cx_set_get(&rt->fields, f)) {
    cx_error(s->cx, s->cx->row, s->cx->col,
	     "Invalid %s field: %s",
	     rt->imp.id, f->id);
    
    return false;
  }
  
  struct cx_box *v = cx_rec_get(r->as_ptr, *f);

  if (v) {
    cx_copy(cx_push(s), v);
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }

  return true;
}

static bool put_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 2));
  struct cx_sym *fid = &cx_test(cx_call_arg(call, 1))->as_sym;
  struct cx_box *r = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  struct cx_rec_type *rt = cx_baseof(r->type, struct cx_rec_type, imp);
  struct cx_field *f = cx_set_get(&rt->fields, fid);

  if (!f) {
    cx_error(s->cx, s->cx->row, s->cx->col,
	     "Invalid %s field: %s",
	     rt->imp.id, fid->id);
    
    return false;
  }

  if (v->type != s->cx->nil_type && !cx_is(v->type, f->type)) {
    cx_error(s->cx, s->cx->row, s->cx->col,
	     "Expected %s, was %s",
	     f->type->id, v->type->id);
    
    return false;
  }

  cx_copy(cx_rec_put(r->as_ptr, *fid), v);
  return true;
}

static bool put_call_imp(struct cx_call *call) {
  struct cx_box *act = cx_test(cx_call_arg(call, 2));
  struct cx_sym *fid = &cx_test(cx_call_arg(call, 1))->as_sym;
  struct cx_box *r = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  struct cx_rec_type *rt = cx_baseof(r->type, struct cx_rec_type, imp);
  struct cx_field *f = cx_set_get(&rt->fields, fid);
    
  if (!f) {
    cx_error(s->cx, s->cx->row, s->cx->col,
	     "Invalid %s field: %s",
	     rt->imp.id, fid->id);
    
    return false;
  }

  struct cx_box *v = cx_rec_get(r->as_ptr, *fid);

  if (v) {
    cx_copy(cx_push(s), v);
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }
      
  if (!cx_call(act, s)) { return false; }
  v = cx_pop(s, false);
  if (!v) { return false; }
  
  if (v->type != s->cx->nil_type && !cx_is(v->type, f->type)) {
    cx_error(s->cx, s->cx->row, s->cx->col,
	     "Expected %s, was %s",
	     f->type->id, v->type->id);
    
    cx_box_deinit(v);
    return false;
  }

  *cx_rec_put(r->as_ptr, *fid) = *v;
  return true;
}

static bool into_imp(struct cx_call *call) {
  struct cx_box
    *in = cx_test(cx_call_arg(call, 0)),
    *outv = cx_test(cx_call_arg(call, 1)),
    it;
  
  struct cx_scope *s = call->scope;
  struct cx_rec_type *rt = cx_baseof(outv->type, struct cx_rec_type, imp);
  cx_iter(in, &it);
  struct cx_rec *out = outv->as_ptr;
  bool ok = false;
  struct cx_box p;

  while (cx_iter_next(it.as_iter, &p, s)) {
    if (!cx_is(p.as_pair->a.type, s->cx->sym_type)) {
      cx_error(s->cx, s->cx->row, s->cx->col,
	       "Expected Sym field id, actual type: %s",
	       p.as_pair->a.type->id);
      
      goto exit;
    }

    struct cx_sym *fid = &p.as_pair->a.as_sym;
    struct cx_field *f = cx_set_get(&rt->fields, fid);
    
    if (!f) {
      cx_error(s->cx, s->cx->row, s->cx->col,
	       "Invalid %s field: %s",
	       rt->imp.id, fid->id);
      
      goto exit;
    }

    if (!cx_is(p.as_pair->b.type, f->type)) {
      cx_error(s->cx, s->cx->row, s->cx->col,
	       "Expected type %s, actual: %s",
	       f->type->id, p.as_pair->b.type->id);
      
      goto exit;
    }

    cx_copy(cx_rec_put(out, *fid), &p.as_pair->b);
    cx_box_deinit(&p);
  }

  cx_copy(cx_push(s), outv);
  ok = true;
 exit:
  cx_box_deinit(&it);
  return ok;
}

static bool eqval_imp(struct cx_call *call) {
  struct cx_rec
    *x = cx_test(cx_call_arg(call, 1))->as_ptr,
    *y = cx_test(cx_call_arg(call, 0))->as_ptr;

  struct cx_scope *s = call->scope;
  bool ok = false;
  if (x->fields.count != y->fields.count) { goto exit; }

  cx_do_env(&x->fields, xv) {
    struct cx_box *yv = cx_rec_get(y, xv->id);
    if (!yv || !cx_eqval(&xv->value, yv)) { goto exit; }
  }

  ok = true;
 exit:
  cx_box_init(cx_push(s), s->cx->bool_type)->as_bool = ok;
  return true;
}

static bool ok_imp(struct cx_call *call) {
  struct cx_rec *r = cx_test(cx_call_arg(call, 0))->as_ptr;
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->bool_type)->as_bool = r->fields.count;
  return true;
}

static bool print_imp(struct cx_call *call) {
  struct cx_box
    *r = cx_test(cx_call_arg(call, 0)),
    *out = cx_test(cx_call_arg(call, 1));
  cx_dump(r, cx_file_ptr(out->as_file));
  return true;
}

cx_lib(cx_init_rec, "cx/rec") { 
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Bool", "Cmp", "Opt", "Sym") ||
      !cx_use(cx, "cx/io", "WFile") ||
      !cx_use(cx, "cx/type", "new")) {
    return false;
  }

  cx->rec_type = cx_add_type(lib, "Rec", cx->cmp_type);
  cx->rec_type->meta = CX_TYPE_REC;

  cx_add_rmacro(lib, "rec:", rec_parse); 

  cx_add_cfunc(lib, "=",
	       cx_args(cx_arg("x", cx->rec_type), cx_arg("y", cx->rec_type)),
	       cx_args(cx_arg(NULL, cx->bool_type)),
	       eqval_imp);

  cx_add_cfunc(lib, "?",
	       cx_args(cx_arg("rec", cx->rec_type)),
	       cx_args(cx_arg(NULL, cx->bool_type)),
	       ok_imp);

  cx_add_cfunc(lib, "get",
	       cx_args(cx_arg("rec", cx->rec_type), cx_arg("fld", cx->sym_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       get_imp);

  cx_add_cfunc(lib, "put",
	       cx_args(cx_arg("rec", cx->rec_type),
		       cx_arg("fld", cx->sym_type),
		       cx_arg("val", cx->opt_type)),
	       cx_args(),
	       put_imp);

  cx_add_cfunc(lib, "put-call",
	       cx_args(cx_arg("rec", cx->rec_type),
		       cx_arg("fld", cx->sym_type),
		       cx_arg("act", cx->any_type)),
	       cx_args(),
	       put_call_imp);

  cx_add_cfunc(lib, "->",
	       cx_args(cx_arg("in", cx_type_get(cx->seq_type, cx->pair_type)),
		       cx_arg("out", cx->rec_type)),
	       cx_args(cx_narg(cx, NULL, 1)),
	       into_imp);

  cx_add_cfunc(lib, "print",
	       cx_args(cx_arg("rec", cx->rec_type), cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       print_imp);

  return true;
}
