#ifndef CX_OP_H
#define CX_OP_H

#include "cixl/box.h"

#define cx_op_type(id, ...)			\
  struct cx_op_type *id() {			\
    static struct cx_op_type type;		\
    static bool init = true;			\
						\
    if (init) {					\
      init = false;				\
      cx_op_type_init(&type, #id);		\
      __VA_ARGS__;				\
    }						\
						\
    return &type;				\
  }						\

struct cx_func;
struct cx_fimp;
struct cx_op;
struct cx_tok;

struct cx_op_type {
  const char *id;
  bool (*eval)(struct cx_op *, struct cx_tok *, struct cx *);
};

struct cx_op_type *cx_op_type_init(struct cx_op_type *type, const char *id);

struct cx_begin_op {
  bool child;
};

struct cx_end_op {
  bool push_result;
};

struct cx_get_const_op {
  struct cx_sym id;
};

struct cx_get_var_op {
  struct cx_sym id;
};

struct cx_fimp_op {
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

struct cx_put_arg_op {
  struct cx_sym id;
};

struct cx_put_var_op {
  struct cx_sym id;
  struct cx_type *type;
};

struct cx_op {
  size_t tok_idx;
  struct cx_op_type *type;
  
  union {
    struct cx_begin_op as_begin;
    struct cx_end_op as_end;
    struct cx_get_const_op as_get_const;
    struct cx_get_var_op as_get_var;
    struct cx_fimp_op as_fimp;
    struct cx_funcall_op as_funcall;
    struct cx_lambda_op as_lambda;
    struct cx_put_arg_op as_put_arg;
    struct cx_put_var_op as_put_var;
  };
};

struct cx_op *cx_op_init(struct cx_op *op, struct cx_op_type *type, size_t tok_idx);

struct cx_op_type *CX_OBEGIN();
struct cx_op_type *CX_OCUT();
struct cx_op_type *CX_OEND();
struct cx_op_type *CX_OGET_CONST();
struct cx_op_type *CX_OGET_VAR();
struct cx_op_type *CX_OFIMP();
struct cx_op_type *CX_OFUNCALL();
struct cx_op_type *CX_OLAMBDA();
struct cx_op_type *CX_OPUSH();
struct cx_op_type *CX_OPUT_ARG();
struct cx_op_type *CX_OPUT_VAR();
struct cx_op_type *CX_ORETURN();
struct cx_op_type *CX_OSTASH();
struct cx_op_type *CX_OSTOP();
struct cx_op_type *CX_OZAP();
struct cx_op_type *CX_OZAP_ARG();

#endif
