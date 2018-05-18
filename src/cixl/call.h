#ifndef CX_CALL_H
#define CX_CALL_H

#include "cixl/arg.h"
#include "cixl/box.h"

struct cx_call {
  int row, col;
  struct cx_fimp *fimp;
  struct cx_scope *scope;
  struct cx_box args[CX_MAX_ARGS];
  int recalls;
};

struct cx_call *cx_call_init(struct cx_call *c,
			     int row, int col,
			     struct cx_fimp *fimp,
			     struct cx_scope *scope);

struct cx_call *cx_call_deinit(struct cx_call *c);
struct cx_box *cx_call_arg(struct cx_call *c, unsigned int i);
bool cx_call_pop_args(struct cx_call *c);
void cx_call_deinit_args(struct cx_call *c);
struct cx_call *cx_call_copy(struct cx_call *dst, struct cx_call *src);

#endif
