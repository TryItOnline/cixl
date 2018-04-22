#ifndef CX_STACK_H
#define CX_STACK_H

#include "cixl/vec.h"

struct cx;
struct cx_type;

struct cx_stack {
  struct cx *cx;
  struct cx_vec imp;
  unsigned int nrefs;
};

struct cx_stack *cx_stack_new(struct cx *cx);
struct cx_stack *cx_stack_ref(struct cx_stack *stack);
void cx_stack_deref(struct cx_stack *stack);
void cx_stack_dump(struct cx_vec *imp, FILE *out);

struct cx_iter *cx_stack_iter_new(struct cx_stack *stack,
				  ssize_t start, size_t end,
				  int delta);

struct cx_type *cx_init_stack_type(struct cx_lib *lib);

#endif
