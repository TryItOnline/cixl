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
  bool (*emit)(struct cx_op *, struct cx_bin *, FILE *, struct cx *);

  bool (*error_eval)(struct cx_op *, struct cx_bin *, struct cx *);
  bool (*error_emit)(struct cx_op *, struct cx_bin *, FILE *, struct cx *);

  void (*emit_init)(struct cx_op *, struct cx_bin *, FILE *, struct cx *);
  void (*emit_labels)(struct cx_op *, struct cx_bin *, struct cx_set *, struct cx *);
  void (*emit_funcs)(struct cx_op *, struct cx_set *, struct cx *);
  void (*emit_fimps)(struct cx_op *, struct cx_set *, struct cx *);
  void (*emit_syms)(struct cx_op *, struct cx_set *, struct cx *);
  void (*emit_types)(struct cx_op *, struct cx_set *, struct cx *);
  void (*emit_libs)(struct cx_op *, struct cx_bin *, struct cx_set *, struct cx *);
};

struct cx_op_type *cx_op_type_init(struct cx_op_type *type, const char *id);

struct cx_argref_op {
  struct cx_type *type;
};

struct cx_begin_op {
  bool child;
  struct cx_fimp *fimp;
  ssize_t nops;
};

struct cx_catch_op {
  struct cx_type *type;
  ssize_t nops;
};

struct cx_else_op {
  ssize_t nops;
};

struct cx_fimp_op {
  struct cx_fimp *imp;
  bool init;
};

struct cx_funcdef_op {
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
  ssize_t pc;
};

struct cx_lambda_op {
  ssize_t start_op, nops;
};

struct cx_libdef_op {
  struct cx_lib *lib;
  ssize_t init;
};

struct cx_push_op {
  struct cx_box value;
};

struct cx_pushlib_op {
  struct cx_lib *lib;
};

struct cx_putargs_op {
  struct cx_fimp *imp;
};

struct cx_putconst_op {
  struct cx_lib *lib;
  struct cx_sym id;
};

struct cx_putvar_op {
  struct cx_sym id;
  struct cx_type *type;
};

struct cx_return_op {
  struct cx_fimp *imp;
  ssize_t pc;
};

struct cx_typedef_op {
  struct cx_type *type;
};

struct cx_op {
  struct cx_op_type *type;
  ssize_t tok_idx, pc;
  int row, col;
  
  union {
    struct cx_argref_op   as_argref;
    struct cx_begin_op    as_begin;
    struct cx_catch_op    as_catch;
    struct cx_else_op     as_else;
    struct cx_fimp_op     as_fimp;
    struct cx_funcdef_op  as_funcdef;
    struct cx_funcall_op  as_funcall;
    struct cx_getconst_op as_getconst;
    struct cx_getvar_op   as_getvar;
    struct cx_jump_op     as_jump;
    struct cx_lambda_op   as_lambda;
    struct cx_libdef_op   as_libdef;
    struct cx_push_op     as_push;
    struct cx_pushlib_op  as_pushlib;
    struct cx_putargs_op  as_putargs;
    struct cx_putconst_op as_putconst;
    struct cx_putvar_op   as_putvar;
    struct cx_return_op   as_return;
    struct cx_typedef_op  as_typedef;
  };
};

struct cx_op *cx_op_new(struct cx_bin *bin,
			struct cx_op_type *type,
			ssize_t tok_idx);

struct cx_op *cx_op_init(struct cx_op *op,
			 struct cx_op_type *type,
			 ssize_t tok_idx,
			 ssize_t pc);

struct cx_op *cx_op_deinit(struct cx_op *o);


struct cx_op_type *CX_OARGREF();
struct cx_op_type *CX_OBEGIN();
struct cx_op_type *CX_OCATCH();
struct cx_op_type *CX_OEND();
struct cx_op_type *CX_OELSE();
struct cx_op_type *CX_OFIMP();
struct cx_op_type *CX_OFUNCDEF();
struct cx_op_type *CX_OFUNCALL();
struct cx_op_type *CX_OGETCONST();
struct cx_op_type *CX_OGETVAR();
struct cx_op_type *CX_OJUMP();
struct cx_op_type *CX_OLAMBDA();
struct cx_op_type *CX_OLIBDEF();
struct cx_op_type *CX_OPOPLIB();
struct cx_op_type *CX_OPUSH();
struct cx_op_type *CX_OPUSHLIB();
struct cx_op_type *CX_OPUTARGS();
struct cx_op_type *CX_OPUTCONST();
struct cx_op_type *CX_OPUTVAR();
struct cx_op_type *CX_ORETURN();
struct cx_op_type *CX_OSTASH();
struct cx_op_type *CX_OTYPEDEF();
struct cx_op_type *CX_OUSE();
#endif
