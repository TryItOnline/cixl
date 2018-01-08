#ifndef CX_TYPE_FIMP_H
#define CX_TYPE_FIMP_H

#include <cixl/vec.h>
#include "cixl/type.h"

struct cx;
struct cx_func;
struct cx_scope;

struct cx_fimp {
  struct cx_func *func;
  char *id;
  size_t i;
  struct cx_vec args;
  bool (*ptr)(struct cx_scope *);
  struct cx_vec toks;
  struct cx_scope *scope;
  struct cx_bin *bin;
};

struct cx_fimp *cx_fimp_init(struct cx_fimp *imp,
			     struct cx_func *func,
			     char *id,
			     size_t i);

struct cx_fimp *cx_fimp_deinit(struct cx_fimp *imp);
bool cx_fimp_match(struct cx_fimp *imp, struct cx_vec *stack, struct cx_scope *scope);
bool cx_fimp_eval(struct cx_fimp *imp, struct cx_scope *scope);
bool cx_fimp_call(struct cx_fimp *imp, struct cx_scope *scope);

struct cx_type *cx_init_fimp_type(struct cx *cx);

#endif
