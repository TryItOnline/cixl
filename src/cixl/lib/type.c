#include <ctype.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/type.h"
#include "cixl/mfile.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/type_set.h"

static ssize_t type_set_eval(struct cx_rmacro_eval *eval,
			    struct cx_bin *bin,
			    size_t tok_idx,
			    struct cx *cx) {
  struct cx_tok *t = cx_vec_get(&eval->toks, 0);
  
  cx_op_new(bin,
	    CX_OTYPEDEF(),
	    tok_idx)->as_typedef.type = t->as_ptr;

  return tok_idx+1;
}

static bool type_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  int row = cx->row, col = cx->col;
  bool ok = false;
  
  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing type id");
    goto exit2;
  }

  struct cx_tok id_tok = *(struct cx_tok *)cx_vec_pop(&toks);

  if (id_tok.type != CX_TID()) {
    cx_error(cx, row, col, "Invalid type: %s", id_tok.type->id);
    goto exit1;
  }

  struct cx_type *type = cx_get_type(cx, id_tok.as_ptr, true);
  
  if (type && type->meta != CX_TYPE) {
    cx_error(cx, row, col, "Attempt to redefine %s as type", type->id);
    goto exit1;
  }
  
  if (!cx_parse_end(cx, in, &toks, true)) {
    if (!cx->errors.count) { cx_error(cx, cx->row, cx->col, "Missing type end"); }
    goto exit1;
  }

  cx_do_vec(&toks, struct cx_tok, t) {
    if (t->type != CX_TID()) {
      cx_error(cx, row, col, "Invalid type arg");
      goto exit1;
    }
  }

  struct cx_type_set *ts = NULL;
  
  if (type) {
    cx_type_reinit(type);
    ts = cx_baseof(type, struct cx_type_set, imp);
    cx_set_clear(&ts->set);
  } else {
    ts = cx_type_set_new(*cx->lib, id_tok.as_ptr, true);
    type = &ts->imp;
    cx_type_push_args(type, cx->opt_type);
    if (!cx_lib_push_type(*cx->lib, type)) { goto exit1; }
    type->meta = CX_TYPE;
    type->type_init = cx_type_init_imp;
  }

  cx_do_vec(&toks, struct cx_tok, t) {
    struct cx_type *mt = cx_get_type(cx, t->as_ptr, false);
    if (!mt) { goto exit1; }

    struct cx_type **ok = cx_set_insert(&ts->set, &mt);
    
    if (!ok) {
      cx_error(cx, t->row, t->col, "Duplicate member in type %s: %s",
	       type->id, mt->id);
      
      goto exit1;
    }

    *ok = mt;
    struct cx_type *tt = cx_type_get(type, mt);
    cx_derive(tt, mt);
    cx_type_define_conv(tt, mt);
  }

  struct cx_rmacro_eval *eval = cx_rmacro_eval_new(type_set_eval);
  cx_tok_init(cx_vec_push(&eval->toks), CX_TTYPE(), row, col)->as_ptr = type;
  cx_tok_init(cx_vec_push(out), CX_TRMACRO(), row, col)->as_ptr = eval;
  ok = true;
 exit1:
  cx_tok_deinit(&id_tok);
 exit2: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

static bool type_id_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  int row = cx->row, col = cx->col;
  bool ok = false;
  
  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing type id");
    goto exit1;
  }

  struct cx_tok id_tok = *cx_test((struct cx_tok *)cx_vec_pop(&toks));

  if (id_tok.type != CX_TID()) {
    cx_error(cx, row, col, "Invalid type id: %s", id_tok.type->id);
    goto exit2;
  }

  struct cx_type *type = cx_get_type(cx, id_tok.as_ptr, true);
  
  if (type && type->meta != CX_TYPE_ID) {
    cx_error(cx, row, col, "Attempt to redefine %s as type id", type->id);
    goto exit2;
  }

  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing type-id: parents");
    goto exit2;
  }

  struct cx_vec parents;
  cx_vec_init(&parents, sizeof(struct cx_type *));
  struct cx_tok parents_tok = *cx_test((struct cx_tok *)cx_vec_peek(&toks, 0));

  if (parents_tok.type == CX_TGROUP()) {
    cx_do_vec(&parents_tok.as_vec, struct cx_tok, t) {
      if (t->type != CX_TID()) {
	cx_error(cx, row, col, "Invalid type id parent: %s", t->type->id);
	goto exit3;
      }

      struct cx_type *pt = cx_get_type(cx, t->as_ptr, false);
      if (!pt) { goto exit3; }

      *(struct cx_type **)cx_vec_push(&parents) = pt;
    }

    cx_vec_pop(&toks);
    cx_tok_deinit(&parents_tok);
  }

  if (!cx_parse_end(cx, in, &toks, true)) {
    if (!cx->errors.count) { cx_error(cx, cx->row, cx->col, "Missing type id end"); }
    goto exit3;
  }

  cx_do_vec(&toks, struct cx_tok, t) {
    if (t->type != CX_TID()) {
      cx_error(cx, row, col, "Invalid type id arg: %s", t->type->id);
      goto exit3;
    }
  }
  
  struct cx_type_set *ts = NULL;

  if (type) {
    cx_type_reinit(type);
    ts = cx_baseof(type, struct cx_type_set, imp);
    cx_set_clear(&ts->set);
  } else {
    ts = cx_type_set_new(*cx->lib, id_tok.as_ptr, true);
    type = &ts->imp;
    if (!cx_lib_push_type(*cx->lib, type)) { goto exit3; }
    type->meta = CX_TYPE_ID;
    type->type_init = cx_type_id_init_imp;
  }

  cx_derive(type, cx->any_type);
  
  cx_do_vec(&parents, struct cx_type *, pt) {
    struct cx_type **ok = cx_set_insert(&ts->parents, pt);

    if (!ok) {
      cx_error(cx, row, col, "Duplicate parent type: %s", (*pt)->id);
      goto exit3;
    }

    *ok = *pt;
    if (!cx_type_has_refs(*pt)) { cx_derive(type, *pt); }
  }
  
  cx_do_vec(&toks, struct cx_tok, t) {
    struct cx_type *ct = cx_get_type(cx, t->as_ptr, false);
    if (!ct) { goto exit3; }
    struct cx_type **ok = cx_set_insert(&ts->set, &ct);
    
    if (!ok) {
      cx_error(cx, t->row, t->col, "Duplicate member in type id %s: %s",
	       type->id, ct->id);
      
      goto exit3;
    }

    *ok = ct;
    
    if (!cx_type_has_refs(ct)) {
      cx_derive(ct, type);

      cx_do_set(&ts->parents, struct cx_type *, pt) {
	if (!cx_is(ct, *pt)) {
	  cx_error(cx, t->row, t->col,
		   "Type %s is not compatible with %s",
		   ct->id, (*pt)->id);
	  
	  goto exit3;
	}
      }
    }
  }

  if (ts->set.members.count == 1) {
    struct cx_type **mt = cx_vec_get(&ts->set.members, 0);

    if (!cx_type_has_refs(*(struct cx_type **)cx_vec_get(&ts->set.members, 0))) {
      struct cx_type **ok = cx_test(cx_set_insert(&ts->parents, mt));
      *ok = *mt;
      cx_type_copy(type, *mt);
      cx_derive(type, *mt);
    }
  }

  struct cx_rmacro_eval *eval = cx_rmacro_eval_new(type_set_eval);
  cx_tok_init(cx_vec_push(&eval->toks), CX_TTYPE(), row, col)->as_ptr = type;
  cx_tok_init(cx_vec_push(out), CX_TRMACRO(), row, col)->as_ptr = eval;
  ok = true;
 exit3:
  cx_vec_deinit(&parents);
 exit2:
  cx_tok_deinit(&id_tok);
 exit1: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

static bool type_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), cx_type_get(s->cx->meta_type, v->type))->as_ptr = v->type;
  return true;
}

static bool is_imp(struct cx_call *call) {
  struct cx_type
    *y = cx_test(cx_call_arg(call, 1))->as_ptr,
    *x = cx_test(cx_call_arg(call, 0))->as_ptr;
  
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->bool_type)->as_bool = cx_is(x, y);
  return true;
}

static bool new_imp(struct cx_call *call) {
  struct cx_type *t = cx_test(cx_call_arg(call, 0))->as_ptr;
  struct cx_scope *s = call->scope;
  
  if (!t->new) {
    cx_error(s->cx, s->cx->row, s->cx->col, "%s does not implement new", t->id);
    return false;
  }
  
  struct cx_box *v = cx_push(s);
  v->type = t;
  t->new(v);
  return true;
}

static bool lib_imp(struct cx_call *call) {
  struct cx_type *t = cx_test(cx_call_arg(call, 0))->as_ptr;
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->lib_type)->as_lib = t->lib;
  return true;
}

static bool safe_imp(struct cx_call *call) {
  call->scope->safe = true;
  return true;
}

static bool unsafe_imp(struct cx_call *call) {
  call->scope->safe = false;
  return true;
}

cx_lib(cx_init_type, "cx/type") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Bool", "Lib", "Opt")) {
    return false;
  }

  cx_add_rmacro(lib, "type:", type_parse);
  cx_add_rmacro(lib, "type-id:", type_id_parse);

  cx_add_cfunc(lib, "type",
	       cx_args(cx_arg("v", cx->opt_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->meta_type,
						cx_arg_ref(cx, 0, 0)))),
	       type_imp);
  
  cx_add_cfunc(lib, "is",
	       cx_args(cx_arg("x", cx->meta_type), cx_arg("y", cx->meta_type)),
	       cx_args(cx_arg(NULL, cx->bool_type)),
	       is_imp);

  cx_add_cxfunc(lib, "is",
		cx_args(cx_arg("x", cx->opt_type), cx_arg("y", cx->meta_type)),
		cx_args(cx_arg(NULL, cx->bool_type)),
		"$x type $y is");
    
  cx_add_cfunc(lib, "new",
	       cx_args(cx_arg("t", cx->meta_type)),
	       cx_args(cx_narg(cx, NULL, 0, 0)),
	       new_imp);

  cx_add_cfunc(lib, "lib",
	       cx_args(cx_arg("t", cx->meta_type)),
	       cx_args(cx_arg(NULL, cx->lib_type)),
	       lib_imp);

  cx_add_cfunc(lib, "safe", cx_args(), cx_args(), safe_imp);
  cx_add_cfunc(lib, "unsafe", cx_args(), cx_args(), unsafe_imp);

  return true;
}
