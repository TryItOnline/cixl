#ifndef CX_TYPE_H
#define CX_TYPE_H

#include <stdint.h>
#include <stdio.h>
#include "cixl/set.h"

struct cx;
struct cx_box;
struct cx_scope;

typedef uint64_t cx_type_tag_t;

struct cx_type {
  struct cx *cx;
  char *id;
  cx_type_tag_t tag, tags;
  struct cx_set parents;
  bool trait;
  
  bool (*eqval)(struct cx_box *, struct cx_box *);
  bool (*equid)(struct cx_box *, struct cx_box *);
  bool (*call)(struct cx_box *, struct cx_scope *);
  bool (*ok)(struct cx_box *);
  void (*copy)(struct cx_box *dst, struct cx_box *src);
  void (*fprint)(struct cx_box *, FILE *);
  void (*deinit)(struct cx_box *);
};

struct cx_type *cx_type_init(struct cx_type *type, struct cx *cx, const char *id);
struct cx_type *cx_type_deinit(struct cx_type *type);
void cx_derive(struct cx_type *child, struct cx_type *parent);
bool cx_is(struct cx_type *child, struct cx_type *parent);

struct cx_type *cx_init_meta_type(struct cx *cx);

#endif
