#ifndef CX_SCOPE_H
#define CX_SCOPE_H

#include <stdio.h>
#include "cixl/env.h"
#include "cixl/vec.h"

struct cx;
struct cx_scan;

struct cx_scope {
  struct cx *cx;
  struct cx_vec parents;
  struct cx_vec stack;
  struct cx_env vars;
  
  bool safe;
  unsigned int nrefs;
};

struct cx_scope *cx_scope_new(struct cx *cx, struct cx_scope *parent);
struct cx_scope *cx_scope_ref(struct cx_scope *scope);
void cx_scope_deref(struct cx_scope *scope);

struct cx_box *cx_push(struct cx_scope *scope);
struct cx_box *cx_pop(struct cx_scope *scope, bool silent);
struct cx_box *cx_peek(struct cx_scope *scope, bool silent);

struct cx_box *cx_get_var(struct cx_scope *scope, struct cx_sym id, bool silent);
struct cx_box *cx_put_var(struct cx_scope *scope, struct cx_sym id);

void cx_stash(struct cx_scope *s);
void cx_reset(struct cx_scope *s);

#endif
