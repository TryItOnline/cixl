#include <ctype.h>
#include <inttypes.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lambda.h"
#include "cixl/lib.h"
#include "cixl/lib/func.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/stack.h"

static bool parse_args(struct cx *cx, struct cx_vec *toks, struct cx_vec *args) {
  struct cx_vec tmp_ids;
  cx_vec_init(&tmp_ids, sizeof(struct cx_tok));
  bool ok = false;

  cx_do_vec(toks, struct cx_tok, t) {
    if (t->type == CX_TID()) {
      char *id = t->as_ptr;

      if (id[0] == '#') {
	struct cx_box *v = cx_get_const(*cx->lib, cx_sym(cx, id+1), false);
	if (!v) { goto exit; }
	*(struct cx_arg *)cx_vec_push(args) = cx_varg(v);
      } else if (strncmp(id, "Arg", 3) == 0 && isdigit(id[3])) {
	int i = strtoimax(id+3, NULL, 10);

	if (tmp_ids.count) {
	  cx_do_vec(&tmp_ids, struct cx_tok, id) {
	    const char *n = strcmp(id->as_ptr, "_") ? id->as_ptr : NULL;
	    *(struct cx_arg *)cx_vec_push(args) = cx_narg(n, i);
	  }
	  
	  cx_vec_clear(&tmp_ids);
	} else {
	  *(struct cx_arg *)cx_vec_push(args) = cx_narg(NULL, i);      
	}	
      } else {
	*(struct cx_tok *)cx_vec_push(&tmp_ids) = *t;
      }
    } else if (t->type == CX_TLITERAL()) {
      *(struct cx_arg *)cx_vec_push(args) = cx_varg(&t->as_box);        
    } else if (t->type == CX_TTYPE()) {
      struct cx_type *type = t->as_ptr;

      if (tmp_ids.count) {
	cx_do_vec(&tmp_ids, struct cx_tok, id) {
	  const char *n = strcmp(id->as_ptr, "_") ? id->as_ptr : NULL;
	  *(struct cx_arg *)cx_vec_push(args) = cx_arg(n, type);      
	}
      
	cx_vec_clear(&tmp_ids);
      } else {
	struct cx_box v;
	cx_box_init(&v, cx->meta_type)->as_ptr = type;
	*(struct cx_arg *)cx_vec_push(args) = cx_varg(&v);
      }
    } else {
      cx_error(cx, t->row, t->col, "Unexpected tok: %d", t->type);
      goto exit;
    }
  }

  cx_do_vec(&tmp_ids, struct cx_tok, id) {
    *(struct cx_arg *)cx_vec_push(args) = cx_arg(id->as_ptr, cx->any_type);      
  }
  
  ok = true;
 exit:
  cx_vec_deinit(&tmp_ids);
  return ok;
}

static ssize_t func_eval(struct cx_macro_eval *eval,
			 struct cx_bin *bin,
			 size_t tok_idx,
			 struct cx *cx) {
  struct cx_tok *f = cx_vec_get(&eval->toks, 0);
  
  cx_op_init(bin,
	     CX_OFUNCDEF(),
	     tok_idx)->as_funcdef.imp = f->as_ptr;

  cx_fimp_inline(f->as_ptr, tok_idx, bin, cx);
  return tok_idx+1;
}

static bool func_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  int row = cx->row, col = cx->col;
  
  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing func id");
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }

  struct cx_tok id = *(struct cx_tok *)cx_vec_pop(&toks);

  if (id.type != CX_TID()) {
    cx_error(cx, row, col, "Invalid func id: %s", id.type->id);
    cx_tok_deinit(&id);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }

  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing func args");
    cx_tok_deinit(&id);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }

  struct cx_tok args = *(struct cx_tok *)cx_vec_pop(&toks);

  if (args.type != CX_TGROUP()) {
    cx_error(cx, row, col, "Invalid func args: %s", args.type->id);
    cx_tok_deinit(&id);
    cx_tok_deinit(&args);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }
  
  struct cx_vec func_args;
  cx_vec_init(&func_args, sizeof(struct cx_arg));

  if (!parse_args(cx, &args.as_vec, &func_args)) {
    cx_tok_deinit(&id);
    cx_tok_deinit(&args);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    cx_vec_deinit(&func_args);
    return false;  
  }

  cx_tok_deinit(&args);

  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing func rets");
    cx_tok_deinit(&id);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }

  struct cx_tok rets = *(struct cx_tok *)cx_vec_pop(&toks);

  if (rets.type != CX_TGROUP()) {
    cx_error(cx, row, col, "Invalid func rets: %s", rets.type->id);
    cx_tok_deinit(&id);
    cx_tok_deinit(&rets);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }
  
  struct cx_vec func_rets;
  cx_vec_init(&func_rets, sizeof(struct cx_arg));

  if (!parse_args(cx, &rets.as_vec, &func_rets)) {
    cx_tok_deinit(&id);
    cx_tok_deinit(&rets);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    cx_vec_deinit(&func_rets);
    return false;
  }

  cx_tok_deinit(&rets);
  
  if (!cx_parse_end(cx, in, &toks, true)) {
    if (!cx->errors.count) { cx_error(cx, cx->row, cx->col, "Missing func end"); }

    cx_tok_deinit(&id);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    cx_vec_deinit(&func_args);
    return false;
  }

  struct cx_fimp *imp = cx_add_func(*cx->lib,
				    id.as_ptr,
				    func_args.count,
				    (void *)func_args.items,
				    func_rets.count,
				    (void *)func_rets.items);

  cx_tok_deinit(&id);
  cx_vec_deinit(&func_args);
  cx_vec_deinit(&func_rets);

  if (!imp) {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }    
    cx_vec_deinit(&toks);
    return false;
  }
  
  imp->toks = toks;
  struct cx_macro_eval *eval = cx_macro_eval_new(func_eval);
  cx_tok_init(cx_vec_push(&eval->toks), CX_TFIMP(), row, col)->as_ptr = imp;
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  return true;
}

static bool func_id_imp(struct cx_scope *scope) {
  struct cx_func *f = cx_test(cx_pop(scope, false))->as_ptr;
  cx_box_init(cx_push(scope), scope->cx->sym_type)->as_sym = cx_sym(scope->cx, f->id);
  return true;
}

static bool func_lib_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_func *f = cx_test(cx_pop(scope, false))->as_ptr;
  cx_box_init(cx_push(scope), cx->lib_type)->as_lib = f->lib;
  return true;
}

static bool fimp_lib_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_fimp *f = cx_test(cx_pop(scope, false))->as_ptr;
  cx_box_init(cx_push(scope), cx->lib_type)->as_lib = f->lib;
  return true;
}

static bool imps_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_func *f = cx_test(cx_pop(scope, false))->as_ptr;
  struct cx_stack *is = cx_stack_new(cx);

  cx_do_set(&f->imps, struct cx_fimp *, i) {
    cx_box_init(cx_vec_push(&is->imp), cx->fimp_type)->as_ptr = *i;
  }
  
  cx_box_init(cx_push(scope), scope->cx->stack_type)->as_ptr = is;
  return true;
}

static bool call_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  bool ok = cx_call(&v, scope);
  cx_box_deinit(&v);
  return ok;
}

static struct cx_call *get_fimp_call(struct cx *cx) {
  for (struct cx_call *c = cx_vec_peek(&cx->calls, 0);
       c >= (struct cx_call *)cx->calls.items;
       c--) {
    if (!c->target->ptr) { return c; }
  }

  return NULL;
}

static bool recall_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_call *call = get_fimp_call(cx);
  
  if (!call) {
    cx_error(cx, cx->row, cx->col, "Nothing to recall");
    return false;
  }

  call->recalls++;  
  return true;
}

static bool cx_fimp_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  if (cx->calls.count > 1) {
    struct cx_call *c = cx_vec_peek(&cx->calls, 1);
    cx_box_init(cx_push(scope), cx->fimp_type)->as_ptr = c->target;
  } else {
    cx_box_init(cx_push(scope), cx->nil_type);
  }
  
  return true;
}

cx_lib(cx_init_func, "cx/func") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Lib", "Seq", "Stack", "Sym")) {
    return false;
  }

  cx_add_macro(lib, "func:", func_parse);

  cx_add_cfunc(lib, "id",
	       cx_args(cx_arg("f", cx->func_type)),
	       cx_args(cx_arg(NULL, cx->sym_type)),
	       func_id_imp);

  cx_add_cfunc(lib, "lib",
	       cx_args(cx_arg("f", cx->func_type)),
	       cx_args(cx_arg(NULL, cx->lib_type)),
	       func_lib_imp);

  cx_add_cfunc(lib, "lib",
	       cx_args(cx_arg("f", cx->fimp_type)),
	       cx_args(cx_arg(NULL, cx->lib_type)),
	       fimp_lib_imp);

  cx_add_cfunc(lib, "imps",
	       cx_args(cx_arg("f", cx->func_type)),
	       cx_args(cx_arg(NULL, cx->stack_type)),
	       imps_imp);
  
  cx_add_cfunc(lib, "call",
	       cx_args(cx_arg("act", cx->any_type)),
	       cx_args(),
	       call_imp);

  cx_add_cfunc(lib, "cx-fimp",
	       cx_args(),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       cx_fimp_imp);
  
  
  cx_add_cfunc(lib, "recall", cx_args(), cx_args(), recall_imp);

  return true;
}
