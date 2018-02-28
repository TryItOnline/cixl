#ifndef CX_LIB_SYM_H
#define CX_LIB_SYM_H

struct cx;
struct cx_lib;

struct cx_lib *cx_init_sym(struct cx *cx);
struct cx_lib *cx_init_sym_types(struct cx *cx);

#endif
