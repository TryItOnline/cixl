#ifndef CX_OP_H
#define CX_OP_H

#include <stdbool.h>

#include "cixl/box.h"

#define CX_TAB "    "

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

struct cx_call;
struct cx_func;
struct cx_fimp;
struct cx_op;
struct cx_tok;
  
struct cx_op_type {
  const char *id;
  
  void (*init)(struct cx_op *, struct cx_tok *);
  void (*deinit)(struct cx_op *);

  bool (*eval)(struct cx_op *, struct cx_bin *, struct cx *);
  bool (*emit)(struct cx_op *, struct cx_bin *, FILE *out, struct cx *);

  struct cx_func *(*emit_func)(struct cx_op *);
  struct cx_fimp *(*emit_fimp)(struct cx_op *);
  void (*emit_syms)(struct cx_op *, struct cx_vec *);
  void (*emit_types)(struct cx_op *, struct cx_vec *);
};

struct cx_op_type *cx_op_type_init(struct cx_op_type *type, const char *id);

struct cx_begin_op {
  bool child;
  struct cx_fimp *fimp;
};

struct cx_else_op {
  size_t nops;
};

struct cx_fimp_op {
  struct cx_fimp *imp;
  size_t start_op, nops;
  bool inline1;
};

struct cx_fimpdef_op {
  struct cx_fimp *imp;
};

struct cx_funcall_op {
  struct cx_func *func;
  struct cx_fimp *imp;
};

struct cx_getconst_op {
  struct cx_sym id;
};

struct cx_getvar_op {
  struct cx_sym id;
};

struct cx_jump_op {
  size_t nops;
};

struct cx_lambda_op {
  size_t start_op, nops;
};

struct cx_push_op {
  struct cx_box value;
};

struct cx_putargs_op {
  struct cx_fimp *imp;
};

struct cx_putvar_op {
  struct cx_sym id;
  struct cx_type *type;
};

struct cx_return_op {
  struct cx_fimp *imp;
  size_t pc;
};

struct cx_op {
  struct cx_op_type *type;
  size_t tok_idx, pc;
  int row, col;
  
  union {
    struct cx_begin_op as_begin;
    struct cx_else_op as_else;
    struct cx_fimp_op as_fimp;
    struct cx_fimpdef_op as_fimpdef;
    struct cx_funcall_op as_funcall;
    struct cx_getconst_op as_getconst;
    struct cx_getvar_op as_getvar;
    struct cx_jump_op as_jump;
    struct cx_lambda_op as_lambda;
    struct cx_push_op as_push;
    struct cx_putargs_op as_putargs;
    struct cx_putvar_op as_putvar;
    struct cx_return_op as_return;
  };
};

struct cx_op *cx_op_init(struct cx_bin *bin, struct cx_op_type *type, size_t tok_idx);

struct cx_op_type *CX_OBEGIN();
struct cx_op_type *CX_OEND();
struct cx_op_type *CX_OELSE();
struct cx_op_type *CX_OFIMP();
struct cx_op_type *CX_OFIMPDEF();
struct cx_op_type *CX_OFUNCALL();
struct cx_op_type *CX_OGETCONST();
struct cx_op_type *CX_OGETVAR();
struct cx_op_type *CX_OJUMP();
struct cx_op_type *CX_OLAMBDA();
struct cx_op_type *CX_OPUSH();
struct cx_op_type *CX_OPUTARGS();
struct cx_op_type *CX_OPUTVAR();
struct cx_op_type *CX_ORETURN();
struct cx_op_type *CX_OSTASH();
struct cx_op_type *CX_OSTOP();

bool cx_ogetvar(struct cx_sym id, struct cx_scope *scope);

bool cx_oreturn_recall(struct cx_call *call, size_t pc, struct cx *cx);
void cx_oreturn_end(struct cx_scope *scope);

#endif
