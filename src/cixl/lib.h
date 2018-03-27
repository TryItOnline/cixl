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

#define _cx_lib(_fn, cid, id)				\
  bool _fn(struct cx_lib *);				\
							\
  struct cx_lib *cid(struct cx *_cx) {			\
    struct cx_lib *l = cx_add_lib(_cx, id);		\
    cx_lib_push_init(l, cx_lib_ptr(_fn));		\
    return l;						\
  }							\
							\
  bool _fn(struct cx_lib *lib)				\
  
#define cx_lib(cid, id)				\
  _cx_lib(cx_cid(cid, _fn), cid, id)		\

#define cx_lib_use(lib, ...) ({				\
      const char *ids[] = {__VA_ARGS__};		\
      unsigned int nids = sizeof(ids) / sizeof(char *);	\
      cx_lib_vuse(lib, nids, ids);			\
    })							\

#define cx_use(cx, lib_id, ...) ({			\
      const char *ids[] = {__VA_ARGS__};		\
      unsigned int nids = sizeof(ids) / sizeof(char *);	\
      cx_vuse(cx, lib_id, nids, ids);			\
    })							\

struct cx;
struct cx_arg;
struct cx_lib;

typedef bool (*cx_lib_init_ptr_t)(struct cx_lib *);
  
struct cx_lib_init {
  cx_lib_init_ptr_t ptr;
  struct cx_bin *bin;
  size_t start_pc, nops;
  bool done;
};

struct cx_lib_init cx_lib_ptr(cx_lib_init_ptr_t ptr);
struct cx_lib_init cx_lib_ops(struct cx_bin *bin, size_t start_pc, size_t nops);

struct cx_lib {
  struct cx *cx;
  struct cx_sym id;
  char *emit_id;
  struct cx_vec inits;

  struct cx_set types, macros, funcs;
  struct cx_env consts;
};

struct cx_lib *cx_lib_init(struct cx_lib *lib, struct cx *cx, struct cx_sym id);
struct cx_lib *cx_lib_deinit(struct cx_lib *lib);

void cx_lib_push_init(struct cx_lib *lib, struct cx_lib_init init);

struct cx_type *_cx_add_type(struct cx_lib *lib, const char *id, ...);
struct cx_type *cx_vadd_type(struct cx_lib *lib, const char *id, va_list parents);
struct cx_rec_type *cx_add_rec_type(struct cx_lib *lib, const char *id);
struct cx_type *cx_get_type(struct cx *cx, const char *id, bool silent);

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

struct cx_func *cx_get_func(struct cx *cx, const char *id, bool silent);

struct cx_box *cx_get_const(struct cx_lib *lib, struct cx_sym id, bool silent);
struct cx_box *cx_put_const(struct cx_lib *lib, struct cx_sym id, bool force);

bool cx_lib_vuse(struct cx_lib *lib, unsigned int nids, const char **ids);

bool cx_vuse(struct cx *cx, const char *lib_id,
	     unsigned int nids, const char *ids[]);

struct cx_type *cx_init_lib_type(struct cx_lib *lib);

#endif
