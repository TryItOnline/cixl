#ifndef CX_OP_H
#define CX_OP_H

#include <stdbool.h>

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
  struct cx_scope *parent;
};

struct cx_end_op {
  bool push_result;
};

struct cx_getconst_op {
  struct cx_sym id;
};

struct cx_getvar_op {
  struct cx_sym id;
};

struct cx_fimp_op {
  struct cx_fimp *imp;
  size_t start_op, num_ops;
  bool inline1;
};

struct cx_fimpdef_op {
  struct cx_fimp *imp;
};

struct cx_funcall_op {
  struct cx_func *func;
  struct cx_fimp *imp, *jit_imp;
};

struct cx_lambda_op {
  size_t start_op, num_ops;
};

struct cx_putargs_op {
  struct cx_fimp *imp;
};

struct cx_putvar_op {
  struct cx_sym id;
  struct cx_type *type;
};

struct cx_return_op {
  size_t start_op;
  struct cx_fimp *imp;
};

struct cx_op {
  size_t tok_idx;
  struct cx_op_type *type;
  
  union {
    struct cx_begin_op as_begin;
    struct cx_end_op as_end;
    struct cx_getconst_op as_getconst;
    struct cx_getvar_op as_getvar;
    struct cx_fimp_op as_fimp;
    struct cx_fimpdef_op as_fimpdef;
    struct cx_funcall_op as_funcall;
    struct cx_lambda_op as_lambda;
    struct cx_putargs_op as_putargs;
    struct cx_putvar_op as_putvar;
    struct cx_return_op as_return;
  };
};

struct cx_op *cx_op_init(struct cx_op *op, struct cx_op_type *type, size_t tok_idx);

struct cx_op_type *CX_OBEGIN();
struct cx_op_type *CX_OCUT();
struct cx_op_type *CX_OEND();
struct cx_op_type *CX_OGETCONST();
struct cx_op_type *CX_OGETVAR();
struct cx_op_type *CX_OFIMP();
struct cx_op_type *CX_OFIMPDEF();
struct cx_op_type *CX_OFUNCALL();
struct cx_op_type *CX_OLAMBDA();
struct cx_op_type *CX_OPUSH();
struct cx_op_type *CX_OPUTARGS();
struct cx_op_type *CX_OPUTVAR();
struct cx_op_type *CX_ORETURN();
struct cx_op_type *CX_OSTASH();
struct cx_op_type *CX_OSTOP();

#endif
