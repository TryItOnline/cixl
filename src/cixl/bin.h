#ifndef CX_BIN_H
#define CX_BIN_H

#include <stdio.h>

#include "cixl/set.h"
#include "cixl/vec.h"

struct cx;
struct cx_fimp;
struct cx_tok;

struct cx_bin_func {
  struct cx_fimp *imp;
  size_t start_pc;
};

struct cx_bin {
  struct cx_vec toks, ops;
  struct cx_set funcs;
  unsigned int nrefs;
  bool (*eval)(struct cx *);
};

struct cx_bin *cx_bin_new();

struct cx_bin *cx_bin_init(struct cx_bin *bin);
struct cx_bin *cx_bin_deinit(struct cx_bin *bin);

struct cx_bin *cx_bin_ref(struct cx_bin *bin);
void cx_bin_deref(struct cx_bin *bin);

struct cx_bin_func *cx_bin_add_func(struct cx_bin *bin,
				    struct cx_fimp *imp,
				    size_t op_idx);

struct cx_bin_func *cx_bin_get_func(struct cx_bin *bin, struct cx_fimp *imp);

bool cx_compile(struct cx *cx,
		struct cx_tok *start,
		struct cx_tok *end,
		struct cx_bin *out);

bool cx_eval(struct cx_bin *bin, size_t start_pc, struct cx *cx);
bool cx_emit(struct cx_bin *bin, FILE *out, struct cx *cx);

#endif
