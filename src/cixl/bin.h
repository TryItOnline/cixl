#ifndef CX_BIN_H
#define CX_BIN_H

#include "cixl/vec.h"

struct cx;

struct cx_bin {
  struct cx_vec toks, ops;
  int nrefs;
};

struct cx_bin *cx_bin_new();
struct cx_bin *cx_bin_init(struct cx_bin *bin);
struct cx_bin *cx_bin_deinit(struct cx_bin *bin);

bool cx_compile(struct cx *cx, struct cx_vec *in, struct cx_bin *out);

#endif
