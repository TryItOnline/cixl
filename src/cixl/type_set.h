#ifndef CX_TYPE_SET_H
#define CX_TYPE_SET_H

#include "cixl/type.h"

struct cx_type_set {
  struct cx_type imp;
  struct cx_set set;

  bool (*type_init)(struct cx_type *, int nargs, struct cx_type *args[]);
};

struct cx_type_set *cx_type_set_new(struct cx_lib *lib,
				    const char *id,
				    bool raw);

bool cx_type_id_init_imp(struct cx_type *t, int nargs, struct cx_type *args[]);

#endif
