#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/libs/type.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

static bool trait_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = false;
  
  if (!cx_parse_tok(cx, in, &toks, false)) {
    cx_error(cx, row, col, "Missing trait id");
    goto exit2;
  }

  struct cx_tok id_tok = *(struct cx_tok *)cx_vec_pop(&toks);
  struct cx_type *type = NULL;

  if (id_tok.type == CX_TTYPE()) {
    type = id_tok.as_ptr;
    
    if (!type->trait) {
      cx_error(cx, row, col, "Attempt to redefine %s as trait", type->id);
      goto exit1;
    }
  }

  if (id_tok.type != CX_TID() && id_tok.type != CX_TTYPE()) {
    cx_error(cx, row, col, "Invalid trait id");
    goto exit1;
  }

  if (!cx_parse_end(cx, in, &toks, false)) {
    if (!cx->errors.count) { cx_error(cx, cx->row, cx->col, "Missing trait end"); }
    goto exit1;
  }

  cx_do_vec(&toks, struct cx_tok, t) {
    if (t->type != CX_TTYPE()) {
      cx_error(cx, row, col, "Invalid trait arg");
      goto exit1;
    }
  }

  if (type) {
    cx_type_reinit(type);
  } else {
    type = cx_add_type(cx, id_tok.as_ptr);
    if (!type) { goto exit1; }
    type->trait = true;
  }
  
  cx_do_vec(&toks, struct cx_tok, t) {
    struct cx_type *child = t->as_ptr;
    cx_derive(child, type);
  }

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

void cx_init_type(struct cx *cx) {
  cx_add_macro(cx, "trait:", trait_parse);

  cx_add_cfunc(cx, "type",
	       cx_args(cx_arg("v", cx->opt_type)),
	       cx_rets(cx_ret(cx->meta_type)),
	       type_imp);
  
  cx_add_cxfunc(cx, "is",
		cx_args(cx_arg("x", cx->opt_type), cx_arg("y", cx->meta_type)),
		cx_rets(cx_ret(cx->bool_type)),
		"$x type is $y");

  cx_add_cfunc(cx, "is",
	       cx_args(cx_arg("x", cx->meta_type), cx_arg("y", cx->meta_type)),
	       cx_rets(cx_ret(cx->bool_type)),
	       is_imp);
}
