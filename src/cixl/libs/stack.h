#ifndef CX_LIB_STACK_H
#define CX_LIB_STACK_H

struct cx;
struct cx_lib;

struct cx_lib *cx_init_stack(struct cx *cx);
struct cx_lib *cx_init_stack_types(struct cx *cx);
struct cx_lib *cx_init_stack_ops(struct cx *cx);

#endif
