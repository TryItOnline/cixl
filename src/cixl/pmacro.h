#ifndef CX_PMACRO_H
#define CX_PMACRO_H

#include <stddef.h>

struct cx;
struct cx_bin;
struct cx_pmacro;

typedef ssize_t (*cx_pmacro_eval_t)(struct cx_bin *bin,
				    size_t tok_idx,
				    struct cx *cx);

struct cx_pmacro {
  char *id;
  cx_pmacro_eval_t eval;
};

struct cx_pmacro *cx_pmacro_init(struct cx_pmacro *m,
				 const char *id,
				 cx_pmacro_eval_t eval);

struct cx_pmacro *cx_pmacro_deinit(struct cx_pmacro *m);

#endif
