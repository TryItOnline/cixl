#ifndef CX_BIN_H
#define CX_BIN_H

#include <stdio.h>

#include "cixl/set.h"
#include "cixl/vec.h"

struct cx;
struct cx_fimp;
struct cx_lib;
struct cx_tok;

struct cx_bin {
  struct cx_vec toks, ops;
  
  size_t init_offs;
  unsigned int nrefs;
  bool (*eval)(struct cx *);
};

struct cx_bin *cx_bin_new();

struct cx_bin *cx_bin_init(struct cx_bin *bin);
struct cx_bin *cx_bin_deinit(struct cx_bin *bin);

struct cx_bin *cx_bin_ref(struct cx_bin *bin);
void cx_bin_deref(struct cx_bin *bin);

void cx_init_ops(struct cx_bin *bin);

bool cx_compile(struct cx *cx,
		struct cx_tok *start,
		struct cx_tok *end,
		struct cx_bin *out);

bool cx_eval(struct cx_bin *bin, size_t start_pc, struct cx *cx);
bool cx_eval_toks(struct cx *cx, struct cx_vec *in);
bool cx_eval_str(struct cx *cx, const char *in);
bool cx_emit(struct cx_bin *bin, FILE *out, struct cx *cx);

struct cx_type *cx_init_bin_type(struct cx_lib *lib);

#endif
