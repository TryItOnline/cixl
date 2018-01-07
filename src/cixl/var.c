#include <stdlib.h>
#include <string.h>

#include "cixl/var.h"

struct cx_var *cx_var_init(struct cx_var *var, struct cx_sym id) {
  var->id = id;
  return var;
}

struct cx_var *cx_var_deinit(struct cx_var *var) {
  cx_box_deinit(&var->value);
  return var;
}
