#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/type.h"
#include "cixl/op.h"
#include "cixl/scope.h"

static ssize_t trait_eval(struct cx_macro_eval *eval,
			  struct cx_bin *bin,
			  size_t tok_idx,
			  struct cx *cx) {
  struct cx_tok *t = cx_vec_get(&eval->toks, 0);
  
  cx_op_init(bin,
	     CX_OTYPEDEF(),
	     tok_idx)->as_typedef.type = t->as_ptr;

  return tok_idx+1;
}

static bool trait_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  int row = cx->row, col = cx->col;
  bool ok = false;
  
  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing trait id");
    goto exit2;
  }

  struct cx_tok id_tok = *(struct cx_tok *)cx_vec_pop(&toks);

  if (id_tok.type != CX_TID()) {
    cx_error(cx, row, col, "Invalid trait id: %s", id_tok.type->id);
    goto exit1;
  }

  struct cx_type *type = cx_get_type(*cx->lib, id_tok.as_ptr, true);
  
  if (type && !type->trait) {
    cx_error(cx, row, col, "Attempt to redefine %s as trait", type->id);
    goto exit1;
  }
  
  if (!cx_parse_end(cx, in, &toks, false)) {
    if (!cx->errors.count) { cx_error(cx, cx->row, cx->col, "Missing trait end"); }
    goto exit1;
  }

  cx_do_vec(&toks, struct cx_tok, t) {
    if (t->type != CX_TID()) {
      cx_error(cx, row, col, "Invalid trait arg");
      goto exit1;
    }
  }

  if (type) {
    cx_type_reinit(type);
  } else {
    type = cx_add_type(*cx->lib, id_tok.as_ptr);
    if (!type) { goto exit1; }
    type->trait = true;
  }
  
  cx_do_vec(&toks, struct cx_tok, t) {
    struct cx_type *child = cx_get_type(*cx->lib, t->as_ptr, false);
    if (!child) { goto exit1; }
    cx_derive(child, type);
  }

  struct cx_macro_eval *eval = cx_macro_eval_new(trait_eval);
  cx_tok_init(cx_vec_push(&eval->toks), CX_TTYPE(), row, col)->as_ptr = type;
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  ok = true;
 exit1:
  cx_tok_deinit(&id_tok);
 exit2: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

static bool type_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->meta_type)->as_ptr = v.type;
  return true;
}

static bool is_imp(struct cx_scope *scope) {
  struct cx_type
    *y = cx_test(cx_pop(scope, false))->as_ptr,
    *x = cx_test(cx_pop(scope, false))->as_ptr;

  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = cx_is(x, y);
  return true;
}

static bool new_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_type *t = cx_test(cx_pop(scope, false))->as_ptr;

  if (!t->new) {
    cx_error(cx, cx->row, cx->col, "%s does not implement new", t->id);
    return false;
  }
  
  struct cx_box *v = cx_push(scope);
  v->type = t;
  t->new(v);
  return true;
}

static bool lib_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_type *t = cx_test(cx_pop(scope, false))->as_ptr;
  cx_box_init(cx_push(scope), cx->lib_type)->as_lib = t->lib;
  return true;
}

static bool safe_imp(struct cx_scope *scope) {
  scope->safe = true;
  return true;
}

static bool unsafe_imp(struct cx_scope *scope) {
  scope->safe = false;
  return true;
}

cx_lib(cx_init_type, "cx/type") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Bool", "Lib", "Opt")) {
    return false;
  }

  cx_add_macro(lib, "trait:", trait_parse);

  cx_add_cfunc(lib, "type",
	       cx_args(cx_arg("v", cx->opt_type)),
	       cx_args(cx_arg(NULL, cx->meta_type)),
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
	       cx_args(cx_arg(NULL, cx->any_type)),
	       new_imp);

  cx_add_cfunc(lib, "lib",
	       cx_args(cx_arg("t", cx->meta_type)),
	       cx_args(cx_arg(NULL, cx->lib_type)),
	       lib_imp);

  cx_add_cfunc(lib, "safe", cx_args(), cx_args(), safe_imp);
  cx_add_cfunc(lib, "unsafe", cx_args(), cx_args(), unsafe_imp);

  return true;
}
