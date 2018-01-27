#ifndef CX_TYPE_VECT_H
#define CX_TYPE_VECT_H

#include "cixl/vec.h"

struct cx;
struct cx_type;

struct cx_vect {
  struct cx *cx;
  struct cx_vec imp;
  unsigned int nrefs;
};

struct cx_vect *cx_vect_new();
struct cx_vect *cx_vect_ref(struct cx_vect *vect);
void cx_vect_deref(struct cx_vect *vect);
void cx_vect_dump(struct cx_vec *imp, FILE *out);

struct cx_type *cx_init_vect_type(struct cx *cx);

#endif
