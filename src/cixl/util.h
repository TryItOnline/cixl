#ifndef CX_UTIL_H
#define CX_UTIL_H

#include <stdarg.h>

#define cx_min(x, y) ({				\
      typeof(x) _x = x;				\
      typeof(x) _y = y;				\
      (_x <= _y) ? _x : _y;			\
    })						\

char *cx_vfmt(const char *spec, va_list args);

#endif
