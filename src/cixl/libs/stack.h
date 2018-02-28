#ifndef CX_LIB_STACK_H
#define CX_LIB_STACK_H

#include "cixl/vec.h"

struct cx;

struct cx_stack {
  struct cx *cx;
  struct cx_vec imp;
  unsigned int nrefs;
};

struct cx_stack *cx_stack_new();
struct cx_stack *cx_stack_ref(struct cx_stack *stack);
void cx_stack_deref(struct cx_stack *stack);
void cx_stack_dump(struct cx_vec *imp, FILE *out);

struct cx_lib *cx_init_stack(struct cx *cx);
struct cx_lib *cx_init_stack_types(struct cx *cx);
struct cx_lib *cx_init_stack_ops(struct cx *cx);

#endif
