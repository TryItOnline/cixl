#ifndef CX_OP_H
#define CX_OP_H

#include "cixl/box.h"

struct cx_func;
struct cx_func_imp;
struct cx_op;
struct cx_tok;

enum cx_op_type {CX_OCUT, CX_OGET, CX_OFUNCALL, CX_OLAMBDA, CX_OPUSH, CX_OSCOPE,
		 CX_OSET, CX_OSTOP, CX_OUNSCOPE};

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

struct cx_set_op {
  char *id;
  bool parent, force;
};

struct cx_op {
  size_t tok_idx;
  enum cx_op_type type;
  
  union {
    struct cx_get_op as_get;
    struct cx_funcall_op as_funcall;
    struct cx_lambda_op as_lambda;
    struct cx_set_op as_set;
    struct cx_scope_op as_scope;
  };
};

struct cx_op *cx_op_init(struct cx_op *op, enum cx_op_type type, size_t tok_idx);
bool cx_op_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx);

#endif
