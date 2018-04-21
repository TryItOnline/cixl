#ifndef CX_FUNC_H
#define CX_FUNC_H

#include <stdarg.h>

#include "cixl/set.h"

struct cx_arg;
struct cx_fimp;
struct cx_scope;
struct cx_type;

struct cx_func {
  struct cx_lib *lib;
  char *id, *emit_id;
  struct cx_set imps;
  int nargs;
};

struct cx_func *cx_func_init(struct cx_func *func,
			     struct cx_lib *lib,
			     const char *id,
			     int nargs);

struct cx_func *cx_func_deinit(struct cx_func *func);

bool cx_ensure_fimp(struct cx_func *func, struct cx_fimp *imp);

struct cx_fimp *cx_add_fimp(struct cx_func *func,
			    int nargs, struct cx_arg *args,
			    int nrets, struct cx_arg *rets);

struct cx_fimp *cx_get_fimp(struct cx_func *func,
			    const char *id,
			    bool silent);

struct cx_fimp *cx_func_match(struct cx_func *func, struct cx_scope *scope);
struct cx_type *cx_init_func_type(struct cx_lib *lib);

#endif
