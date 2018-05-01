#ifndef CX_TYPE_SET_H
#define CX_TYPE_SET_H

#include "cixl/type.h"

struct cx_type_set {
  struct cx_type imp;
  struct cx_set  parents, set;
};

struct cx_type_set *cx_type_set_new(struct cx_lib *lib,
				    const char *id,
				    bool raw);

bool cx_type_id_init_imp(struct cx_type *t, int nargs, struct cx_type *args[]);
bool cx_type_init_imp(struct cx_type *t, int nargs, struct cx_type *args[]);
void cx_type_define_conv(struct cx_type *t, struct cx_type *mt);

bool cx_type_id_init_imp(struct cx_type *t, int nargs, struct cx_type *args[]);
bool cx_type_init_imp(struct cx_type *t, int nargs, struct cx_type *args[]);

#endif
