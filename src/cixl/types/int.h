#ifndef CX_TYPE_INT_H
#define CX_TYPE_INT_H

#include <stdint.h>

struct cx;
struct cx_type;

typedef int64_t cx_int_t;

#define cx_abs(x) ({				\
      typeof(x) _x = x;				\
      _x < 0 ? -_x : _x;			\
    })						\

struct cx_type *cx_init_int_type(struct cx *cx);

#endif
