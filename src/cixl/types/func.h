#ifndef CX_TYPE_FUNC_H
#define CX_TYPE_FUNC_H

#include <stdarg.h>

#include "cixl/box.h"
#include "cixl/set.h"
#include "cixl/type.h"

struct cx_fimp;
struct cx_scope;
struct cx_type;

struct cx_func {
  struct cx *cx;
  char *id, *emit_id;
  struct cx_set imp_lookup;
  struct cx_vec imps;
  int nargs;
};

struct cx_func *cx_func_init(struct cx_func *func,
			     struct cx *cx,
			     const char *id,
			     int nargs);

struct cx_func *cx_func_deinit(struct cx_func *func);
			  
struct cx_fimp *cx_add_fimp(struct cx_func *func,
			    int nargs, struct cx_arg *args,
			    int nrets, struct cx_arg *rets);

struct cx_fimp *cx_get_fimp(struct cx_func *func,
			    const char *id,
			    bool silent);

struct cx_fimp *cx_func_match(struct cx_func *func,
			      struct cx_scope *scope,
			      size_t offs);

struct cx_type *cx_init_func_type(struct cx *cx);

#endif
