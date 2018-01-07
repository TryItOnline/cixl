#ifndef CX_SCOPE_H
#define CX_SCOPE_H

#include <stdio.h>
#include "cixl/set.h"
#include "cixl/vec.h"

struct cx;

struct cx_scope {
  struct cx *cx;
  struct cx_scope *parent;
  struct cx_vec stack;
  struct cx_set env;
  size_t cut_offs;
  int nrefs;
};

struct cx_scope *cx_scope_new(struct cx *cx, struct cx_scope *parent);
struct cx_scope *cx_scope_ref(struct cx_scope *scope);
void cx_scope_unref(struct cx_scope *scope);

struct cx_box *cx_push(struct cx_scope *scope);
struct cx_box *cx_pop(struct cx_scope *scope, bool silent);
struct cx_box *cx_peek(struct cx_scope *scope, bool silent);
void cx_fprint_stack(struct cx_scope *scope, FILE *out);

struct cx_box *cx_get(struct cx_scope *scope, struct cx_sym id, bool silent);
struct cx_box *cx_set(struct cx_scope *scope, struct cx_sym id, bool force);

bool cx_islet(struct cx_scope *scope, struct cx_sym id);
void cx_unlet(struct cx_scope *scope, struct cx_sym id);

#endif
