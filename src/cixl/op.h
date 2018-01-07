#ifndef CX_OP_H
#define CX_OP_H

#include "cixl/box.h"

struct cx_func;
struct cx_fimp;
struct cx_op;
struct cx_tok;

#define cx_op_type(id, ...)			\
  struct cx_op_type *id() {			\
    static struct cx_op_type type;		\
    static bool init = true;			\
						\
    if (init) {					\
      init = false;				\
      cx_op_type_init(&type);			\
      __VA_ARGS__;				\
    }						\
						\
    return &type;				\
  }						\

struct cx_op_type {
  bool (*eval)(struct cx_op *, struct cx_tok *, struct cx *);
};

struct cx_op_type *cx_op_type_init(struct cx_op_type *type);

struct cx_get_op {
  const char *id;
};

struct cx_func_op {
  struct cx_fimp *imp;
  size_t start_op, num_ops;
};

struct cx_funcall_op {
  struct cx_func *func;
  struct cx_fimp *imp, *jit_imp;
};

struct cx_lambda_op {
  size_t start_op, num_ops;
};

struct cx_scope_op {
  bool child;
};

struct cx_set_op {
  const char *id;
  struct cx_type *type;
  bool pop_parent, set_parent, force;
};

struct cx_unscope_op {
  bool push_result;
};

struct cx_zap_op {
  bool parent;
};

struct cx_op {
  size_t tok_idx;
  struct cx_op_type *type;
  
  union {
    struct cx_get_op as_get;
    struct cx_func_op as_func;
    struct cx_funcall_op as_funcall;
    struct cx_lambda_op as_lambda;
    struct cx_scope_op as_scope;
    struct cx_set_op as_set;
    struct cx_unscope_op as_unscope;
    struct cx_zap_op as_zap;
  };
};

struct cx_op *cx_op_init(struct cx_op *op, struct cx_op_type *type, size_t tok_idx);

struct cx_op_type *CX_OCUT();
struct cx_op_type *CX_OGET();
struct cx_op_type *CX_OGET_CONST();
struct cx_op_type *CX_OFUNC();
struct cx_op_type *CX_OFUNCALL();
struct cx_op_type *CX_OLAMBDA();
struct cx_op_type *CX_OPUSH();
struct cx_op_type *CX_OSCOPE();
struct cx_op_type *CX_OSET();
struct cx_op_type *CX_OSTASH();
struct cx_op_type *CX_OSTOP();
struct cx_op_type *CX_OUNSCOPE();
struct cx_op_type *CX_OZAP();

#endif
