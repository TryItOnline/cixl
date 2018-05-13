#ifndef CX_RMACRO_H
#define CX_RMACRO_H

#include <stdio.h>
#include "cixl/vec.h"

struct cx;
struct cx_bin;
struct cx_rmacro;
struct cx_rmacro_eval;

typedef bool (*cx_rmacro_parse_t)(struct cx *cx, FILE *in, struct cx_vec *out);

struct cx_rmacro {
  char *id;
  cx_rmacro_parse_t imp;
};

struct cx_rmacro *cx_rmacro_init(struct cx_rmacro *m,
			       const char *id,
			       cx_rmacro_parse_t imp);

struct cx_rmacro *cx_rmacro_deinit(struct cx_rmacro *m);

typedef ssize_t (*cx_rmacro_eval_t)(struct cx_rmacro_eval *e,
				   struct cx_bin *bin,
				   size_t tok_idx,
				   struct cx *cx);

struct cx_rmacro_eval {
  struct cx_vec toks;
  cx_rmacro_eval_t imp;
  unsigned int nrefs;
};

struct cx_rmacro_eval *cx_rmacro_eval_new(cx_rmacro_eval_t imp);
struct cx_rmacro_eval *cx_rmacro_eval_ref(struct cx_rmacro_eval *e);
void cx_rmacro_eval_deref(struct cx_rmacro_eval *e);

#endif
