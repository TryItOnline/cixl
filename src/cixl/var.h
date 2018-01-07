#ifndef CX_VAR_H
#define CX_VAR_H

#include "cixl/box.h"
#include "cixl/types/sym.h"

struct cx_var {
  struct cx_sym id;
  struct cx_box value;
};

struct cx_var *cx_var_init(struct cx_var *var, struct cx_sym id);
struct cx_var *cx_var_deinit(struct cx_var *var);

#endif
