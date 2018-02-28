#ifndef CX_LIB_H
#define CX_LIB_H

#include <stdbool.h>

#include "cixl/types/sym.h"
#include "cixl/vec.h"

#define cx_lib(cid, id, ...)					\
  struct cx_lib *cid(struct cx *cx) {				\
    struct cx_sym sid = cx_sym(cx, id);				\
    struct cx_lib *lib = cx_set_get(&cx->libs, &sid);		\
								\
    if (!lib) {							\
      lib = cx_set_insert(&cx->libs, &sid);			\
      bool fn(struct cx *cx) __VA_ARGS__;			\
      cx_lib_init(lib, sid, fn);				\
    }								\
								\
    return lib;							\
  }								\

struct cx;

typedef bool (*cx_lib_init_t)(struct cx *);

struct cx_lib {
  struct cx_sym id;
  cx_lib_init_t init;
  bool used;
};

struct cx_lib *cx_lib_init(struct cx_lib *lib, struct cx_sym id, cx_lib_init_t init);

#endif
