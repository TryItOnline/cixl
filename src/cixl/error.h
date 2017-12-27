#ifndef CX_ERROR_H
#define CX_ERROR_H

#include <stdio.h>
#include <stdlib.h>

#define cx_test(cond) ({				\
      typeof(cond) _cond = cond;			\
							\
      if (!_cond) {					\
	fprintf(stderr,					\
		"'%s' failed at line %d in %s\n",	\
		#cond, __LINE__, __FILE__);		\
	abort();					\
      }							\
							\
      _cond;						\
    })							\

struct cx;

struct cx_error {
  int row, col;
  char *msg;
};

struct cx_error *cx_error_init(struct cx_error *e, int row, int col, char *msg);
struct cx_error *cx_error_deinit(struct cx_error *e);

struct cx_error *cx_error(struct cx *cx, int row, int col, const char *spec, ...);

#endif
