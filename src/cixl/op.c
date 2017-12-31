#include "cixl/bin.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/func.h"
#include "cixl/types/lambda.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

struct cx_op *cx_op_init(struct cx_op *op, enum cx_op_type type, size_t tok_idx) {
  op->tok_idx = tok_idx;
  op->type = type;
  return op;
}

bool cx_op_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  switch (op->type) {
  case CX_OCUT: {
    struct cx_scope *s = cx_scope(cx, 0);
    s->cut_offs = s->stack.count;
    break;
  }

  case CX_OFUNCALL: {
    struct cx_func *func = op->as_funcall.func;
    
    if (!cx_scan_args(cx, func)) { return false; }
    
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

  case CX_OGET: {
    struct cx_scope *s = cx_scope(cx, 0);
    struct cx_box *v = cx_get(s, op->as_get.id, false);
    if (!v) { return false; }
    cx_copy(cx_push(s), v);
    break;
  }
    
  case CX_OLAMBDA: {
    struct cx_scope *scope = cx_scope(cx, 0);
    struct cx_lambda *l = cx_lambda_init(malloc(sizeof(struct cx_lambda)), scope);
    l->start_op = op->as_lambda.start_op;
    l->num_ops = op->as_lambda.num_ops;
    cx_box_init(cx_push(scope), cx->lambda_type)->as_ptr = l;
    cx->op += l->num_ops;
    break;
  }

  case CX_OPUSH: {
    cx_copy(cx_push(cx_scope(cx, 0)),  &tok->as_box);
    break;
  }

  case CX_OSCOPE: {
    cx_begin(cx, op->as_scope.child);
    break;
  }

  case CX_OSET: {
    struct cx_scope *s = cx_scope(cx, op->as_set.parent ? 1 : 0);
    struct cx_box *v = cx_pop(s, false);
    if (!v) { return false; }
    *cx_set(op->as_set.parent ? cx_scope(cx, 0) : s,
	    op->as_set.id,
	    op->as_set.force) = *v;
    break;
  }

  case CX_OSTOP: {
    cx->stop = true;
    break;
  }

  case CX_OUNSCOPE: {
    cx_end(cx);
    break;
  }

  default:
    cx_error(cx, tok->row, tok->col, "Unknown op: %d", op->type);
    return false;
  }

  return true;
}

