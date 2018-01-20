#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/libs/func.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

static ssize_t func_eval(struct cx_macro_eval *eval,
			 struct cx_bin *bin,
			 size_t tok_idx,
			 struct cx *cx) {
  struct cx_tok *f = cx_vec_get(&eval->toks, 0);

  cx_op_init(cx_vec_push(&bin->ops),
	     CX_OFIMPDEF(),
	     tok_idx)->as_fimpdef.imp = f->as_ptr;
  
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
    cx_error(cx, row, col, "Invalid func id");
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
    cx_error(cx, row, col, "Invalid func args");
    cx_tok_deinit(&id);
    cx_tok_deinit(&args);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return false;
  }
  
  struct cx_vec func_args;
  cx_vec_init(&func_args, sizeof(struct cx_func_arg));

  if (!cx_eval_args(cx, &args.as_vec, &func_args)) {
    cx_tok_deinit(&id);
    cx_tok_deinit(&args);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    cx_vec_deinit(&func_args);
    return false;  
  }

  cx_tok_deinit(&args);
  
  if (!cx_parse_end(cx, in, &toks, true)) {
    if (!cx->errors.count) { cx_error(cx, cx->row, cx->col, "Missing func end"); }

    cx_tok_deinit(&id);
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    cx_vec_deinit(&func_args);
    return false;
  }

  struct cx_fimp *imp = _cx_add_func(cx,
				     id.as_ptr,
				     func_args.count,
				     (void *)func_args.items);
  imp->toks = toks;

  cx_tok_deinit(&id);
  cx_vec_deinit(&func_args);

  struct cx_macro_eval *eval = cx_macro_eval_new(func_eval);
  cx_tok_init(cx_vec_push(&eval->toks), CX_TFIMP(), row, col)->as_ptr = imp;
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  return true;
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

static bool upcall_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_call *call = get_fimp_call(cx);
  
  if (!call) {
    cx_error(cx, cx->row, cx->col, "Nothing to upcall");
    return false;
  }

  struct cx_fimp *imp = call->target;

  if (!imp->i) {
    cx_error(cx, cx->row, cx->col, "No more fimps");
    return false;
  }
  
  struct cx_func *func = imp->func;
  if (!cx_scan_args(cx, func)) { return false; }

  imp = cx_func_get_imp(func,
			&scope->stack,
			func->imps.count - imp->i);
  
  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Upcall not applicable");
    return false;
  }

  return cx_fimp_call(imp, scope);
}

void cx_init_func(struct cx *cx) {
  cx_add_macro(cx, "func:", func_parse);

  cx_add_cfunc(cx, "recall", recall_imp);
  cx_add_cfunc(cx, "upcall", upcall_imp);
}
