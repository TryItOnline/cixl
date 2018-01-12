#ifndef CX_SCOPE_H
#define CX_SCOPE_H

#include <stdio.h>
#include "cixl/set.h"
#include "cixl/vec.h"

struct cx;

struct cx_scope {
  struct cx *cx;
  struct cx_scope *parent;
  struct cx_vec stack, cut_offs;
  struct cx_set env;
  int nrefs;
};

struct cx_scope *cx_scope_new(struct cx *cx, struct cx_scope *parent);
struct cx_scope *cx_scope_ref(struct cx_scope *scope);
void cx_scope_unref(struct cx_scope *scope);

struct cx_box *cx_push(struct cx_scope *scope);
struct cx_box *cx_pop(struct cx_scope *scope, bool silent);
struct cx_box *cx_peek(struct cx_scope *scope, bool silent);
void cx_fprint_stack(struct cx_scope *scope, FILE *out);

struct cx_box *cx_get_var(struct cx_scope *scope, struct cx_sym id, bool silent);
struct cx_box *cx_put_var(struct cx_scope *scope, struct cx_sym id, bool force);
bool cx_del_var(struct cx_scope *scope, struct cx_sym id, bool silent);

#endif
