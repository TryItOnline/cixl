#ifndef CX_TYPE_PAIR_H
#define CX_TYPE_PAIR_H

#include "cixl/box.h"

struct cx;
struct cx_type;

struct cx_pair {
  struct cx_box x, y;
  unsigned int nrefs;
};

struct cx_pair *cx_pair_new(struct cx *cx, struct cx_box *x, struct cx_box *y);

struct cx_pair *cx_pair_ref(struct cx_pair *pair);
void cx_pair_deref(struct cx_pair *pair, struct cx *cx);

struct cx_type *cx_init_pair_type(struct cx *cx);

#endif
