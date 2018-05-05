#ifndef CX_POINT_H
#define CX_POINT_H

struct cx_lib;
struct cx_type;

struct cx_point {
  cx_float_t x, y;
};

struct cx_point *cx_point_init(struct cx_point *p, cx_float_t x, cx_float_t y);
struct cx_type *cx_init_point_type(struct cx_lib *lib);

#endif
