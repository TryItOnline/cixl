#include <stdlib.h>
#include <string.h>

#include "cixl/env.h"

struct cx_var *cx_var_init(struct cx_var *var, struct cx_sym id) {
  var->id = id;
  return var;
}

struct cx_var *cx_var_deinit(struct cx_var *var) {
  cx_box_deinit(&var->value);
  return var;
}

struct cx_env *cx_env_init(struct cx_env *env) {
  cx_vec_init(&env->vars, sizeof(struct cx_var));
  return env;
}

struct cx_env *cx_env_deinit(struct cx_env *env) {
  cx_do_vec(&env->vars, struct cx_var, v) { cx_var_deinit(v); }
  cx_vec_deinit(&env->vars);
  return env;
}

struct cx_box *cx_env_put(struct cx_env *env, struct cx_sym id) {
  struct cx_var *var = cx_var_init(cx_vec_push(&env->vars), id);
  return &var->value;
}

struct cx_box *cx_env_get(struct cx_env *env, struct cx_sym id) {
  if (!env->vars.count) { return NULL; }
  
  for (struct cx_var *v = cx_vec_peek(&env->vars, 0);
       v >= (struct cx_var *)env->vars.items;
       v--) {
    if (v->id.tag == id.tag) { return &v->value; }
  }
  
  return NULL;
}
