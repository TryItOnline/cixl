#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/lib.h"
#include "cixl/lib/error.h"

static ssize_t catch_eval(struct cx_macro_eval *eval,
			struct cx_bin *bin,
			size_t tok_idx,
			struct cx *cx) {
  size_t start_pc = bin->ops.count;
  
  struct cx_tok
    *ts = cx_vec_get(&eval->toks, 0),
    *tt = cx_vec_get(&ts->as_vec, 0);

  unsigned int ncatch = 0;
  
  bool compile(struct cx_vec *toks) {
    size_t i = bin->ops.count;
    struct cx_op *op = cx_op_init(bin, CX_OCATCH(), tok_idx);
    struct cx_tok *t = cx_vec_get(toks, 0);
    op->as_catch.type = t->as_ptr;
    if (!cx_compile(cx, cx_vec_get(toks, 1), cx_vec_end(toks), bin)) { return false; }
    cx_op_init(bin, CX_OJUMP(), tok_idx)->as_jump.pc = -1;
    op = cx_vec_get(&bin->ops, i);
    op->as_catch.nops = bin->ops.count-i-1;
    ncatch++;
    return true;
  } 
    
  if (tt->type == CX_TTYPE()) {
    if (!compile(&ts->as_vec)) { return -1; }
  } else {
    for (struct cx_tok *t = cx_vec_peek(&ts->as_vec, 0);
	 t >= (struct cx_tok *)cx_vec_start(&ts->as_vec);
	 t--) {
      if (!compile(&t->as_vec)) { return -1; }
    }
  }

  if (!cx_compile(cx, cx_vec_get(&eval->toks, 1), cx_vec_end(&eval->toks), bin)) {
    return -1;
  }

  cx_op_init(bin, CX_OPOPCATCH(), tok_idx)->as_popcatch.n = ncatch;

  for (struct cx_op *op = cx_vec_get(&bin->ops, start_pc);
       op != cx_vec_end(&bin->ops);
       op++) {
    if (op->type == CX_OJUMP() && op->as_jump.pc == -1) {
      op->as_jump.pc = bin->ops.count;
    }
  }

  return tok_idx+1;
}

static bool catch_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  struct cx_macro_eval *eval = cx_macro_eval_new(catch_eval);
  
  if (!cx_parse_end(cx, in, &eval->toks, true)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing catch end"); }
    cx_macro_eval_deref(eval);
    return false;
  }

  struct cx_tok
    *ts = cx_vec_get(&eval->toks, 0),
    *tt = cx_vec_get(&ts->as_vec, 0);

  if (tt->type == CX_TTYPE()) {
    // Skip
  } else if (tt->type == CX_TGROUP()) {
    cx_do_vec(&ts->as_vec, struct cx_tok, t) {
      if (t->type != CX_TGROUP()) {
	cx_error(cx, tt->row, tt->col, "Invalid catch clause: %s", t->type->id);
	cx_macro_eval_deref(eval);
	return false;
      }
      
      tt = cx_vec_get(&t->as_vec, 0);

      if (tt->type != CX_TTYPE()) {
	cx_error(cx, tt->row, tt->col, "Invalid catch type: %s", tt->type->id);
	cx_macro_eval_deref(eval);
	return false;
      }
    }
  } else {
    cx_error(cx, tt->row, tt->col, "Invalid catch clause: %s", tt->type->id);
    cx_macro_eval_deref(eval);
    return false;
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

static bool throw_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_throw(scope->cx, &v);
  cx_box_deinit(&v);
  return false;
}

static bool dump_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  if (!cx->throwing.count) {
    cx_error(cx, cx->row, cx->col, "Nothing to dump");
    return false;
  }

  cx_do_vec(&cx->throwing, struct cx_error, e) { cx_error_dump(e, stdout); }
  return true;
}

cx_lib(cx_init_error, "cx/error") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "Opt", "Str") ||
      !cx_use(cx, "cx/cond", "?")) {
    return false;
  }

  cx_add_macro(lib, "catch:", catch_parse);

  cx_add_cfunc(lib, "check",
	       cx_args(cx_arg("v", cx->opt_type)), cx_args(),
	       check_imp);
    
  cx_add_cfunc(lib, "throw",
	       cx_args(cx_arg("e", cx->any_type)), cx_args(),
	       throw_imp);

  cx_add_cfunc(lib, "dump",
	       cx_args(), cx_args(),
	       dump_imp);

  return true;
}
