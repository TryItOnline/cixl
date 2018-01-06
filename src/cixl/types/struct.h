#ifndef CX_STRUCT_H
#define CX_STRUCT_H

#include "cixl/set.h"
#include "cixl/type.h"

struct cx;

struct cx_struct_type {
  struct cx_type imp;
  struct cx_set slot_types;
};

struct cx_struct_type *cx_struct_type_new(struct cx *cx, const char *id);

struct cx_struct_type *cx_struct_type_init(struct cx_struct_type *type,
					   struct cx *cx,
					   const char *id);

struct cx_struct_type *cx_struct_type_deinit(struct cx_struct_type *type);

struct cx_struct {
  struct cx_type *type;
  struct cx_set slots;
  int nrefs;
};

struct cx_slot_type {
  char *id;
  struct cx_type *type;
};

#endif
