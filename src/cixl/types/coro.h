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
  struct cx_tok *pc;
  struct cx_bin *bin;
  struct cx_op *op;
  int nrefs;
  bool done;
};

struct cx_coro *cx_coro_init(struct cx_coro *coro,
			     struct cx *cx,
			     struct cx_scope *scope);
struct cx_coro *cx_coro_deinit(struct cx_coro *coro);

struct cx_type *cx_init_coro_type(struct cx *cx);

#endif
