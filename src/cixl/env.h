#ifndef CX_ENV_H
#define CX_ENV_H

#include "cixl/box.h"
#include "cixl/types/sym.h"
#include "cixl/vec.h"

struct cx_var {
  struct cx_sym id;
  struct cx_box value;
};

struct cx_var *cx_var_init(struct cx_var *var, struct cx_sym id);
struct cx_var *cx_var_deinit(struct cx_var *var);

struct cx_env {
  struct cx_vec vars;
};

struct cx_env *cx_env_init(struct cx_env *env);
struct cx_env *cx_env_deinit(struct cx_env *env);
struct cx_box *cx_env_put(struct cx_env *env, struct cx_sym id);
struct cx_box *cx_env_get(struct cx_env *env, struct cx_sym id);

#endif
