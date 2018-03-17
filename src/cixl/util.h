#ifndef CX_UTIL_H
#define CX_UTIL_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define _cx_cid(x, y)				\
  x ## y					\
  
#define cx_cid(x, y)				\
  _cx_cid(x, y)					\
  
#define cx_gencid(prefix)			\
  cx_cid(prefix, __COUNTER__)			\

#define cx_min(x, y) ({				\
      typeof(x) _x = x;				\
      typeof(y) _y = y;				\
      (_x <= _y) ? _x : _y;			\
    })						\

#define cx_max(x, y) ({				\
      typeof(x) _x = x;				\
      typeof(y) _y = y;				\
      (_x >= _y) ? _x : _y;			\
    })

#define cx_abs(x) ({				\
      typeof(x) _x = x;				\
      _x < 0 ? -_x : _x;			\
    })						\

#define cx_baseof(ptr, typ, fld) ({			\
      const typeof( ((typ *)0)->fld ) *fp = (ptr);	\
      (typ *)((char *)fp - offsetof(typ, fld));		\
    })							\

char *cx_vfmt(const char *spec, va_list args);
char *cx_fmt(const char *spec, ...);
char *cx_get_dir(const char *in, char *out, size_t len);
size_t cx_str_dist(const char *x, const char *y);
void cx_reverse(char *s);
bool cx_get_line(char **out, size_t *len, FILE *in);
const char *cx_home_dir();
bool cx_make_dir(const char *path);

#endif
