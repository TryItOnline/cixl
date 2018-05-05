#ifndef CX_FLOAT_H
#define CX_FLOAT_H

typedef double cx_float_t;

struct cx_lib;
struct cx_type;

enum cx_cmp cx_cmp_float(const void *x, const void *y);
struct cx_type *cx_init_float_type(struct cx_lib *lib);

#endif
