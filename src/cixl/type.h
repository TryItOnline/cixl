#ifndef CX_TYPE_H
#define CX_TYPE_H

#include <stdio.h>
#include "cixl/set.h"

struct cx;
struct cx_box;
struct cx_scope;

struct cx_type {
  char *id;
  struct cx_set parents;
  bool (*call)(struct cx_box *, struct cx_scope *);
  void (*copy)(struct cx_box *dst, struct cx_box *src);
  void (*fprint)(struct cx_box *, FILE *);
  void (*deinit)(struct cx_box *);
};

struct cx_type *cx_type_init(struct cx_type *type, const char *id);
struct cx_type *cx_type_deinit(struct cx_type *type);
void cx_type_add_parent(struct cx_type *type, struct cx_type *parent);
bool cx_type_is(struct cx_type *type, struct cx_type *parent);

struct cx_type *cx_init_meta_type(struct cx *cx);

#endif
