#ifndef CX_CORO_H
#define CX_CORO_H

#include <stdlib.h>
#include "cixl/vec.h"

struct cx;
struct cx_scope;
struct cx_type;

struct cx_coro {
  struct cx_scope *scope;
  struct cx_vec toks;
  ssize_t pc;
  int nrefs;
  bool done;
};

struct cx_coro *cx_coro_init(struct cx_coro *coro, struct cx_scope *scope);
struct cx_coro *cx_coro_deinit(struct cx_coro *coro);

struct cx_type *cx_init_coro_type(struct cx *cx);

#endif
