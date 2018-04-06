#include "cixl/call.h"

struct cx_call *cx_call_init(struct cx_call *call,
			     int row, int col,
			     struct cx_fimp *target) {
  call->row = row;
  call->col = col;
  call->target = target;
  call->recalls = 0;
  return call;
}

struct cx_call *cx_call_deinit(struct cx_call *call) {
  return call;
}
