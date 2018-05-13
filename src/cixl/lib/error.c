#include <string.h>

#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/lib.h"
#include "cixl/lib/error.h"

static ssize_t catch_eval(struct cx_rmacro_eval *eval,
			struct cx_bin *bin,
			size_t tok_idx,
			struct cx *cx) {
  bool compile(struct cx_vec *toks) {
    size_t i = bin->ops.count;
    struct cx_op *op = cx_op_new(bin, CX_OCATCH(), tok_idx);
    struct cx_tok *t = cx_vec_get(toks, 0);
    char *id = t->as_ptr;
    struct cx_type *type = NULL;

    if (strcmp(id, "_") != 0) {
      type = cx_get_type(cx, id, false);
      if (!type) { return false; }
    }
    
    op->as_catch.type = type;
    if (!cx_compile(cx, cx_vec_get(toks, 1), cx_vec_end(toks), bin)) { return false; }
    op = cx_vec_get(&bin->ops, i);
    op->as_catch.nops = bin->ops.count-i-1;
    return true;
  } 

  struct cx_tok *t = cx_vec_get(&eval->toks, 0);

  if (t->type == CX_TID()) {
    if (!compile(&eval->toks)) { return -1; }
  } else {
    for (struct cx_tok *tt = cx_vec_start(&eval->toks);
	 tt != cx_vec_end(&eval->toks) ;
	 tt++) {
      if (!compile(&tt->as_vec)) { return -1; }
    }
  }

  return tok_idx+1;
}

static bool catch_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  struct cx_rmacro_eval *eval = cx_rmacro_eval_new(catch_eval);
  
  if (!cx_parse_end(cx, in, &eval->toks, true)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing catch end"); }
    cx_rmacro_eval_deref(eval);
    return false;
  }

  struct cx_tok *t = cx_vec_get(&eval->toks, 0);

  if (t->type == CX_TID()) {
    // Skip
  } else if (t->type == CX_TGROUP()) {
    cx_do_vec(&eval->toks, struct cx_tok, tt) {
      if (tt->type != CX_TGROUP()) {
	cx_error(cx, tt->row, tt->col, "Invalid catch clause: %s", tt->type->id);
	cx_rmacro_eval_deref(eval);
	return false;
      }
      
      tt = cx_vec_get(&t->as_vec, 0);

      if (tt->type != CX_TID()) {
	cx_error(cx, tt->row, tt->col, "Invalid catch type: %s", tt->type->id);
	cx_rmacro_eval_deref(eval);
	return false;
      }
    }
  } else {
    cx_error(cx, t->row, t->col, "Invalid catch clause: %s", t->type->id);
    cx_rmacro_eval_deref(eval);
    return false;
  }
  
  cx_tok_init(cx_vec_push(out), CX_TRMACRO(), row, col)->as_ptr = eval;
  return true;
}

static bool check_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  if (!cx_ok(v)) { cx_error(s->cx, s->cx->row, s->cx->col, "Check failed"); }
  return true;
}

static bool throw_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  cx_throw(call->scope->cx, v);
  return true;
}

static bool throw_error_imp(struct cx_call *call) {
  struct cx_box *e = cx_test(cx_call_arg(call, 0));
  cx_throw_error(call->scope->cx, e->as_error);  
  return true;
}

static bool value_imp(struct cx_call *call) {
  struct cx_box *e = cx_test(cx_call_arg(call, 0));
  cx_copy(cx_push(call->scope), &e->as_error->value);
  return true;
}

cx_lib(cx_init_error, "cx/error") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "Opt", "Str") ||
      !cx_use(cx, "cx/cond", "?")) {
    return false;
  }

  cx->error_type = cx_init_error_type(lib);
  
  cx_add_rmacro(lib, "catch:", catch_parse);

  cx_add_cfunc(lib, "check",
	       cx_args(cx_arg("v", cx->opt_type)), cx_args(),
	       check_imp);
    
  cx_add_cfunc(lib, "throw",
	       cx_args(cx_arg("e", cx->opt_type)), cx_args(),
	       throw_imp);

  cx_add_cfunc(lib, "throw",
	       cx_args(cx_arg("e", cx->error_type)), cx_args(),
	       throw_error_imp);

  cx_add_cfunc(lib, "value",
	       cx_args(cx_arg("e", cx->error_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       value_imp);

  return true;
}
