#ifndef CX_CATCH_H
#define CX_CATCH_H

#include <stddef.h>
#include "cixl/box.h"

struct cx_catch {
  struct cx_type *type;
  struct cx_bin *bin;
  size_t tok_idx;
  size_t start_pc, nops;
  ssize_t stop_pc;
  size_t lib_offs;
};

struct cx_catch *cx_catch_init(struct cx_catch *c,
			       struct cx_type *type,
			       struct cx_bin *bin,
			       size_t tok_idx,
			       size_t start_pc, size_t nops,
			       ssize_t stop_pc);

struct cx_catch *cx_catch_deinit(struct cx_catch *c);

bool cx_catch_eval(struct cx_catch *c);

#endif
