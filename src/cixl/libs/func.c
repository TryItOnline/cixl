#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/op.h"
#include "cixl/scan.h"
#include "cixl/scope.h"
#include "cixl/libs/func.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/vect.h"

static ssize_t func_eval(struct cx_macro_eval *eval,
			 struct cx_bin *bin,
			 size_t tok_idx,
			 struct cx *cx) {
  struct cx_tok *f = cx_vec_get(&eval->toks, 0);

  cx_op_init(bin,
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
  cx_vec_init(&func_rets, sizeof(struct cx_func_ret));

  if (!cx_eval_rets(cx, &rets.as_vec, &func_args, &func_rets)) {
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

  struct cx_fimp *imp = cx_add_fimp(cx,
				    id.as_ptr,
				    func_args.count,
				    (void *)func_args.items,
				    func_rets.count,
				    (void *)func_rets.items);
  imp->toks = toks;

  cx_tok_deinit(&id);
  cx_vec_deinit(&func_args);
  cx_vec_deinit(&func_rets);

  struct cx_macro_eval *eval = cx_macro_eval_new(func_eval);
  cx_tok_init(cx_vec_push(&eval->toks), CX_TFIMP(), row, col)->as_ptr = imp;
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  return true;
}

static bool imps_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_func *f = cx_test(cx_pop(scope, false))->as_ptr;
  struct cx_vect *is = cx_vect_new();

  for (struct cx_fimp **i = cx_vec_peek(&f->imps, 0);
       i >= (struct cx_fimp **)f->imps.items;
       i--) {
    cx_box_init(cx_vec_push(&is->imp), cx->fimp_type)->as_ptr = *i;
  }
  
  cx_box_init(cx_push(scope), scope->cx->vect_type)->as_ptr = is;
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

static bool upcall_scan(struct cx_scan *scan) {
  struct cx_fimp *imp = scan->as_upcall.imp;
  struct cx_func *func = imp->func;
  struct cx_scope *s = scan->scope;
  struct cx *cx = s->cx;
  
  imp = cx_func_get_imp(func, s, func->imps.count - imp->i);
  
  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Upcall not applicable");
    return false;
  }
  
  return cx_fimp_call(imp, s);  
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
  
  struct cx_scan *scan = cx_scan(scope, imp->func, upcall_scan);
  scan->as_upcall.imp = imp;
  return true;
}

void cx_init_func(struct cx *cx) {
  cx_add_macro(cx, "func:", func_parse);

  cx_add_cfunc(cx, "imps",
	       cx_args(cx_arg("f", cx->func_type)), cx_rets(cx_ret(cx->vect_type)),
	       imps_imp);
  
  cx_add_cfunc(cx, "recall", cx_args(), cx_rets(), recall_imp);
  cx_add_cfunc(cx, "upcall", cx_args(), cx_rets(), upcall_imp);
}
