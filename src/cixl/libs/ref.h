#ifndef CX_LIB_REF_H
#define CX_LIB_REF_H

#include "cixl/box.h"

struct cx;
struct cx_lib;

struct cx_ref {
  struct cx_box value;
  unsigned int nrefs;
};

struct cx_ref *cx_ref_new(struct cx *cx, struct cx_box *value);
struct cx_ref *cx_ref_ref(struct cx_ref *ref);
void cx_ref_deref(struct cx_ref *ref);

struct cx_lib *cx_init_ref(struct cx *cx);
struct cx_lib *cx_init_ref_types(struct cx *cx);

#endif
