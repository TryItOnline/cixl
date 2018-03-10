#ifndef CX_ENV_H
#define CX_ENV_H

#define CX_ENV_SLOTS 5

#define _cx_do_env(_i, env, var)					\
  for (unsigned int _i = 0; _i < CX_ENV_SLOTS; _i++)			\
    for (struct cx_var *var = (env)->slots[_i]; var; var = var->next)	\
      
#define cx_do_env(env, var)			\
  _cx_do_env(cx_gencid(i), env, var)		\

#include "cixl/box.h"
#include "cixl/sym.h"
#include "cixl/vec.h"

struct cx_malloc;

struct cx_var {
  struct cx_sym id;
  struct cx_box value;
  struct cx_var *next;
};

struct cx_env {
  struct cx_var *slots[CX_ENV_SLOTS];
  struct cx_malloc *alloc;
  size_t count;
};

struct cx_env *cx_env_init(struct cx_env *env, struct cx_malloc *alloc);
struct cx_env *cx_env_deinit(struct cx_env *env);

struct cx_var *cx_env_get(struct cx_env *env, struct cx_sym id);
struct cx_box *cx_env_put(struct cx_env *env, struct cx_sym id);
void cx_env_clear(struct cx_env *env);

#endif
