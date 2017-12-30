#ifndef CX_OP_H
#define CX_OP_H

#include "cixl/box.h"

struct cx_func;
struct cx_func_imp;
struct cx_op;
struct cx_tok;

typedef bool (*cx_op_eval_t)(struct cx_tok *, struct cx_op *, struct cx *);

struct cx_get_op {
  char *id;
};

struct cx_funcall_op {
  struct cx_func *func;
  struct cx_func_imp *imp;
};

struct cx_lambda_op {
  size_t start_op, num_ops;
};

struct cx_scope_op {
  bool child;
};

struct cx_op {
  size_t tok_idx;
  cx_op_eval_t eval;
  
  union {
    struct cx_get_op as_get;
    struct cx_funcall_op as_funcall;
    struct cx_lambda_op as_lambda;
    struct cx_scope_op as_scope;
  };
};

struct cx_op *cx_op_init(struct cx_op *op, cx_op_eval_t eval, size_t tok_idx);

bool cx_cut_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx);
bool cx_get_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx);
bool cx_funcall_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx);
bool cx_lambda_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx);
bool cx_macro_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx);
bool cx_push_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx);
bool cx_scope_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx);
bool cx_stop_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx);
bool cx_unscope_op(struct cx_tok *tok, struct cx_op *op, struct cx *cx);

#endif
