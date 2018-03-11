#ifndef CX_REF_H
#define CX_REF_H

#include "cixl/box.h"

struct cx;
struct cx_lib;
struct cx_type;

struct cx_ref {
  struct cx_box value;
  unsigned int nrefs;
};

struct cx_ref *cx_ref_new(struct cx *cx, struct cx_box *value);
struct cx_ref *cx_ref_inc(struct cx_ref *ref);
void cx_ref_dec(struct cx_ref *ref);

struct cx_type *cx_init_ref_type(struct cx_lib *lib);

#endif
