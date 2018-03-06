#ifndef CX_CALL_H
#define CX_CALL_H

#include "cixl/box.h"

struct cx_call {
  int row, col;
  struct cx_fimp *target;
  ssize_t return_pc;
  int recalls;
};

struct cx_call *cx_call_init(struct cx_call *call,
			     int row, int col,
			     struct cx_fimp *target,
			     ssize_t return_pc);

struct cx_call *cx_call_deinit(struct cx_call *call);

#endif
