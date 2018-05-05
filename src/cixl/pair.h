#ifndef CX_PAIR_H
#define CX_PAIR_H

#include "cixl/box.h"

struct cx;
struct cx_lib;
struct cx_type;

struct cx_pair {
  struct cx_box a, b;
  unsigned int nrefs;
};

struct cx_pair *cx_pair_new(struct cx *cx, struct cx_box *a, struct cx_box *b);

struct cx_pair *cx_pair_ref(struct cx_pair *pair);
void cx_pair_deref(struct cx_pair *pair, struct cx *cx);

struct cx_type *cx_init_pair_type(struct cx_lib *lib);

#endif
