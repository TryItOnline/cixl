#ifndef CX_LIB_H
#define CX_LIB_H

#include <stdbool.h>

#include "cixl/env.h"
#include "cixl/fimp.h"
#include "cixl/macro.h"
#include "cixl/set.h"
#include "cixl/sym.h"

#define cx_add_type(cx, id, ...)		\
  _cx_add_type(cx, id, ##__VA_ARGS__, NULL)	\

#define cx_lib(cid, id, ...)				\
  struct cx_lib *cid(struct cx *_cx) {			\
    void init(struct cx_lib *lib) __VA_ARGS__;		\
    struct cx_lib *lib = cx_add_lib(_cx, id, init);	\
    return lib;						\
  }							\

struct cx;
struct cx_arg;
struct cx_lib;

typedef void (*cx_lib_init_t)(struct cx_lib *);

struct cx_lib {
  struct cx *cx;
  struct cx_sym id;
  cx_lib_init_t init;
  
  struct cx_set types, macros, funcs;
  struct cx_env consts;
};

struct cx_lib *cx_lib_init(struct cx_lib *lib,
			   struct cx *cx,
			   struct cx_sym id,
			   cx_lib_init_t init);

struct cx_lib *cx_lib_deinit(struct cx_lib *lib);

struct cx_type *_cx_add_type(struct cx_lib *lib, const char *id, ...);
struct cx_type *cx_vadd_type(struct cx_lib *lib, const char *id, va_list parents);
struct cx_rec_type *cx_add_rec_type(struct cx_lib *lib, const char *id);
struct cx_type *cx_get_type(struct cx_lib *lib, const char *id, bool silent);

struct cx_macro *cx_add_macro(struct cx_lib *lib,
			      const char *id,
			      cx_macro_parse_t imp);

struct cx_macro *cx_get_macro(struct cx_lib *lib, const char *id, bool silent);

struct cx_fimp *cx_add_func(struct cx_lib *lib,
			    const char *id,
			    int nargs, struct cx_arg *args,
			    int nrets, struct cx_arg *rets);

struct cx_fimp *cx_add_cfunc(struct cx_lib *lib,
			     const char *id,
			     int nargs, struct cx_arg *args,
			     int nrets, struct cx_arg *rets,
			     cx_fimp_ptr_t ptr);

struct cx_fimp *cx_add_cxfunc(struct cx_lib *lib,
			      const char *id,
			      int nargs, struct cx_arg *args,
			      int nrets, struct cx_arg *rets,
			      const char *body);

struct cx_func *cx_get_func(struct cx_lib *lib, const char *id, bool silent);

struct cx_box *cx_get_const(struct cx_lib *lib, struct cx_sym id, bool silent);
struct cx_box *cx_set_const(struct cx_lib *lib, struct cx_sym id, bool force);

#endif
