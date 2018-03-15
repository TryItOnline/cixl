#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/lib.h"
#include "cixl/lib/error.h"

static ssize_t catch_eval(struct cx_macro_eval *eval,
			struct cx_bin *bin,
			size_t tok_idx,
			struct cx *cx) {
  //cx_op_init(bin, CX_OCATCH(), tok_idx);
  return tok_idx+1;
}

static bool catch_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  struct cx_macro_eval *eval = cx_macro_eval_new(catch_eval);
  
  if (!cx_parse_end(cx, in, &eval->toks, false)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing catch end"); }
    cx_macro_eval_deref(eval);
    return false;
  }

  cx_do_vec(&eval->toks, struct cx_tok, t) {
    if (t->type == CX_TID()) {
      struct cx_type *et = cx_get_type(*cx->lib, t->as_ptr, false);
      if (!et) {
	cx_macro_eval_deref(eval);
	return false;
      }	
    } else if (t->type == CX_TGROUP()) {
      struct cx_tok *tt = cx_vec_get(&t->as_vec, 0);

      if (tt->type != CX_TID()) {
	cx_error(cx, tt->row, tt->col, "Invalid catch type: %s", tt->type->id);
	cx_macro_eval_deref(eval);
	return false;
      }
    } else {
      cx_error(cx, t->row, t->col, "Invalid catch clause: %s", t->type->id);
      cx_macro_eval_deref(eval);
      return false;
    }
  }
  
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  return true;
}

static bool check_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  struct cx *cx = scope->cx;
  bool ok = true;
  
  if (!cx_ok(&v)) {
    cx_error(cx, cx->row, cx->col, "Check failed");
    ok = false;
  }

  cx_box_deinit(&v);
  return ok;
}

static bool fail_imp(struct cx_scope *scope) {
  struct cx_box m = *cx_test(cx_pop(scope, false));
  struct cx *cx = scope->cx;
  cx_error(cx, cx->row, cx->col, m.as_str->data);
  cx_box_deinit(&m);
  return false;
}

cx_lib(cx_init_error, "cx/error") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "Opt", "Str")) {
    return false;
  }

  cx_add_macro(lib, "catch:", catch_parse);

  cx_add_cfunc(lib, "check",
	       cx_args(cx_arg("v", cx->opt_type)), cx_args(),
	       check_imp);
    
  cx_add_cfunc(lib, "fail",
	       cx_args(cx_arg("msg", cx->str_type)), cx_args(),
	       fail_imp);

  return true;
}
