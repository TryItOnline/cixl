#ifndef CX_ERROR_H
#define CX_ERROR_H

#include <stdio.h>
#include <stdlib.h>

#include "cixl/box.h"
#include "cixl/vec.h"

#define cx_test(cond) ({				\
      typeof(cond) _cond = cond;			\
							\
      if (!_cond) {					\
	fprintf(stderr,					\
		"'%s' failed at line %d in %s\n",	\
		#cond, __LINE__, __FILE__);		\
	exit(-1);					\
      }							\
							\
      _cond;						\
    })							\

struct cx;

struct cx_error {
  int row, col;
  struct cx_vec stack, calls;
  struct cx_box value;
  unsigned int nrefs;
};

struct cx_error *cx_error_init(struct cx_error *e,
			       struct cx *cx,
			       int row, int col,
			       struct cx_box *v);

struct cx_error *cx_error_deinit(struct cx_error *e);

struct cx_error *cx_error_ref(struct cx_error *e);
void cx_error_deref(struct cx_error *e);

void cx_error_dump(struct cx_error *e, FILE *out);

struct cx_error *cx_error(struct cx *cx, int row, int col, const char *spec, ...);
struct cx_error *cx_throw(struct cx *cx, struct cx_box *v);

struct cx_type *cx_init_error_type(struct cx_lib *lib);

#endif
