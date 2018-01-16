#ifndef CX_CALL_ITER_H
#define CX_CALL_ITER_H

struct cx_box;
struct cx_iter;

struct cx_iter *cx_call_iter_new(struct cx_box *target);

#endif
