#ifndef CX_MACRO_H
#define CX_MACRO_H

#include <stdio.h>
#include "cixl/vec.h"

struct cx;
struct cx_bin;
struct cx_macro;
struct cx_macro_eval;

typedef bool (*cx_macro_parse_t)(struct cx *cx, FILE *in, struct cx_vec *out);

struct cx_macro {
  char *id;
  cx_macro_parse_t imp;
};

struct cx_macro *cx_macro_init(struct cx_macro *macro,
			       const char *id,
			       cx_macro_parse_t imp);

struct cx_macro *cx_macro_deinit(struct cx_macro *macro);

typedef ssize_t (*cx_macro_eval_t)(struct cx_macro_eval *eval,
				   struct cx_bin *bin,
				   size_t tok_idx,
				   struct cx *cx);

struct cx_macro_eval {
  struct cx_vec toks;
  cx_macro_eval_t imp;
  int nrefs;
};

struct cx_macro_eval *cx_macro_eval_new(cx_macro_eval_t imp);
struct cx_macro_eval *cx_macro_eval_ref(struct cx_macro_eval *eval);
void cx_macro_eval_unref(struct cx_macro_eval *eval);

#endif
