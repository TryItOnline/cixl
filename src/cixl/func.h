#ifndef CX_FUNC_H
#define CX_FUNC_H

#include <stdarg.h>

#include "cixl/set.h"

struct cx_scope;
struct cx_type;

struct cx_func {
  struct cx *cx;
  char *id;
  struct cx_set imps;
  int nargs;
};

struct cx_func *cx_func_init(struct cx_func *func,
			     struct cx *cx,
			     const char *id,
			     int nargs);

struct cx_func *cx_func_deinit(struct cx_func *func);
			  
struct cx_func_imp {
  struct cx_func *func;
  char *id;
  struct cx_vec args;
  bool (*ptr)(struct cx_scope *);
  struct cx_vec toks;
  struct cx_bin *bin;
};

struct cx_func_imp *cx_func_imp_init(struct cx_func_imp *imp,
				     struct cx_func *func,
				     char *id);

struct cx_func_imp *cx_func_imp_deinit(struct cx_func_imp *imp);
bool cx_func_imp_match(struct cx_func_imp *imp, struct cx_vec *stack);
bool cx_func_imp_call(struct cx_func_imp *imp, struct cx_scope *scope);

struct cx_func_arg {
  struct cx_type *type;
  int narg;
};

struct cx_func_arg cx_arg(struct cx_type *type);
struct cx_func_arg cx_narg(int n);

struct cx_func_imp *cx_func_add_imp(struct cx_func *func,
				    int nargs,
				    struct cx_func_arg *args);

struct cx_func_imp *cx_func_get_imp(struct cx_func *func, struct cx_vec *args);

struct cx_type *cx_init_func_type(struct cx *cx);

#endif
