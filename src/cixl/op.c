#include "cixl/bin.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/func.h"
#include "cixl/types/lambda.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

struct cx_op_type *cx_op_type_init(struct cx_op_type *type) {
  type->eval = NULL;
  return type;
}

struct cx_op *cx_op_init(struct cx_op *op, struct cx_op_type *type, size_t tok_idx) {
  op->tok_idx = tok_idx;
  op->type = type;
  return op;
}

static bool cut_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  s->cut_offs = s->stack.count;
  return true;
}

cx_op_type(CX_OCUT, {
    type.eval = cut_eval;
  });

static bool func_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  cx->op += op->as_func.num_ops;
  return true;
}

cx_op_type(CX_OFUNC, {
    type.eval = func_eval;
  });

static bool funcall_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_func *func = op->as_funcall.func;
  if (!cx_scan_args(cx, func)) { return false; }
    
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_fimp *imp = op->as_funcall.imp;

  if (imp) {
    if (!cx_fimp_match(imp, &s->stack)) { imp = NULL; }
  } else {
    imp = op->as_funcall.jit_imp;
    if (imp && !cx_fimp_match(imp, &s->stack)) { imp = NULL; }
    if (!imp) { imp = cx_func_get_imp(func, &s->stack); }
  }
  
  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: '%s'", func->id);
    return false;
  }
    
  op->as_funcall.jit_imp = imp;
  return cx_fimp_call(imp, s);
}

cx_op_type(CX_OFUNCALL, {
    type.eval = funcall_eval;
  });

static bool get_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_box *v = cx_get(s, op->as_get.id, false);
  if (!v) { return false; }
  cx_copy(cx_push(s), v);
  return true;
}

cx_op_type(CX_OGET, {
    type.eval = get_eval;
  });

static bool lambda_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *scope = cx_scope(cx, 0);
  struct cx_lambda *l = cx_lambda_new(scope,
				      op->as_lambda.start_op,
				      op->as_lambda.num_ops);
  cx_box_init(cx_push(scope), cx->lambda_type)->as_ptr = l;
  cx->op += l->num_ops;
  return true;
}

cx_op_type(CX_OLAMBDA, {
    type.eval = lambda_eval;
  });

static bool push_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  cx_copy(cx_push(cx_scope(cx, 0)),  &tok->as_box);
  return true;
}

cx_op_type(CX_OPUSH, {
    type.eval = push_eval;
  });

static bool scope_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  cx_begin(cx, op->as_scope.child);
  return true;
}

cx_op_type(CX_OSCOPE, {
    type.eval = scope_eval;
  });

static bool set_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, op->as_set.parent ? 1 : 0);
  struct cx_box *src = cx_pop(s, false);
  if (!src) { return false; }
  
  struct cx_box *dst = cx_set(op->as_set.parent ? cx_scope(cx, 0) : s,
			      op->as_set.id,
			      op->as_set.force);

  if (!dst) { return false; }
  *dst = *src;
  return true;
}

cx_op_type(CX_OSET, {
    type.eval = set_eval;
  });

static bool stop_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  cx->stop = true;
  return true;
}

cx_op_type(CX_OSTOP, {
    type.eval = stop_eval;
  });

static bool unscope_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  cx_end(cx);
  return true;
}

cx_op_type(CX_OUNSCOPE, {
    type.eval = unscope_eval;
  });
