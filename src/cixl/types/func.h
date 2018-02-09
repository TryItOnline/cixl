#ifndef CX_TYPE_FUNC_H
#define CX_TYPE_FUNC_H

#include <stdarg.h>

#include "cixl/box.h"
#include "cixl/set.h"
#include "cixl/type.h"

#define cx_args(...)							\
  sizeof((struct cx_func_arg[]){__VA_ARGS__}) /				\
  sizeof(struct cx_func_arg),						\
    (struct cx_func_arg[]){__VA_ARGS__}					\

#define cx_rets(...)							\
  sizeof((struct cx_func_ret[]){__VA_ARGS__}) /				\
  sizeof(struct cx_func_ret),						\
    (struct cx_func_ret[]){__VA_ARGS__}					\

struct cx_fimp;
struct cx_scope;
struct cx_type;

struct cx_func {
  struct cx *cx;
  char *id;
  struct cx_set imp_lookup;
  struct cx_vec imps;
  int nargs;
};

struct cx_func *cx_func_init(struct cx_func *func,
			     struct cx *cx,
			     const char *id,
			     int nargs);

struct cx_func *cx_func_deinit(struct cx_func *func);
			  
struct cx_func_arg {
  char *id;
  struct cx_sym sym_id;
  struct cx_type *type;
  struct cx_box value;
  int narg;
};

struct cx_func_arg *cx_func_arg_deinit(struct cx_func_arg *arg);

struct cx_func_arg cx_arg(const char *id, struct cx_type *type);
struct cx_func_arg cx_varg(struct cx_box *value);
struct cx_func_arg cx_narg(const char *id, int n);

struct cx_func_ret {
  struct cx_type *type;
  int narg;
};

struct cx_func_ret cx_ret(struct cx_type *type);
struct cx_func_ret cx_nret(int n);

struct cx_fimp *cx_func_add_imp(struct cx_func *func,
				int nargs, struct cx_func_arg *args,
				int nrets, struct cx_func_ret *rets);

struct cx_fimp *cx_func_get_imp(struct cx_func *func,
				const char *id,
				bool silent);

struct cx_fimp *cx_func_match_imp(struct cx_func *func,
				  struct cx_scope *scope,
				  size_t offs);

struct cx_type *cx_init_func_type(struct cx *cx);

#endif
