#ifndef CX_VECT_H
#define CX_VECT_H

#include "cixl/vec.h"

struct cx;
struct cx_type;

struct cx_vect {
  struct cx_vec imp;
  int nrefs;
};

struct cx_vect *cx_vect_new();
struct cx_vect *cx_vect_ref(struct cx_vect *vect);
void cx_vect_unref(struct cx_vect *vect);
void cx_vect_fprint(struct cx_vec *imp, FILE *out);

struct cx_type *cx_init_vect_type(struct cx *cx);

#endif
