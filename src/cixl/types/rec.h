#ifndef CX_REC_H
#define CX_REC_H

#include "cixl/set.h"
#include "cixl/type.h"
#include "cixl/types/sym.h"

struct cx;

struct cx_rec_type {
  struct cx_type imp;
  struct cx_set fields;
};

struct cx_field {
  struct cx_sym id;
  struct cx_type *type;
};

struct cx_rec_type *cx_rec_type_new(struct cx *cx, const char *id);

struct cx_rec_type *cx_rec_type_init(struct cx_rec_type *type,
					   struct cx *cx,
					   const char *id);

struct cx_rec_type *cx_rec_type_deinit(struct cx_rec_type *type);

struct cx_rec {
  struct cx_type *type;
  struct cx_set values;
  int nrefs;
};

#endif
