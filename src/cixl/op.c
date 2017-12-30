#include "cixl/bin.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/func.h"
#include "cixl/types/lambda.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

struct cx_op *cx_op_init(struct cx_op *op, cx_op_eval_t eval, size_t tok_idx) {
  op->tok_idx = tok_idx;
  op->eval = eval;
  return op;
}

bool cx_cut_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  s->cut_offs = s->stack.count;
  return true;
}

bool cx_funcall_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx) {
  struct cx_func *func = op->as_funcall.func;
  
  if (!cx_scan_args2(cx, func)) { return false; }
  
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_func_imp *imp = op->as_funcall.imp;
  if (imp && !cx_func_imp_match(imp, &s->stack)) { imp = NULL; }
  if (!imp) { imp = cx_func_get_imp(func, &s->stack); }
  
  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: '%s'", func->id);
    return false;
  }

  op->as_funcall.imp = imp;
  return cx_func_imp_call(imp, s);
}

bool cx_get_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_box *v = cx_get(s, op->as_get.id, false);
  if (!v) { return false; }
  cx_copy(cx_push(s), v);
  return true;
}

bool cx_lambda_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx) {
  struct cx_scope *scope = cx_scope(cx, 0);
  struct cx_lambda *l = cx_lambda_init(malloc(sizeof(struct cx_lambda)), scope);
  l->bin = cx_bin_ref(cx->bin);
  l->start_op = op->as_lambda.start_op;
  l->num_ops = op->as_lambda.num_ops;
  cx_box_init(cx_push(scope), cx->lambda_type)->as_ptr = l;
  cx->op += l->num_ops;
  return true;
}

bool cx_macro_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx) {
  struct cx_macro_eval *eval = tok->as_ptr;
  eval->imp(eval, cx);
  return true;
}

bool cx_push_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx) {
  cx_copy(cx_push(cx_scope(cx, 0)),  &tok->as_box);
  return true;
}

bool cx_scope_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx) {
  cx_begin(cx, op->as_scope.child);
  return true;
}

bool cx_unscope_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx) {
  cx_end(cx);
  return true;
}

