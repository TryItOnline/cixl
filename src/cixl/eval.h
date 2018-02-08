#ifndef CX_EVAL_H
#define CX_EVAL_H

#include <stdbool.h>

struct cx;
struct cx_bin;
struct cx_func;
struct cx_op;
struct cx_vec;

bool cx_eval_str(struct cx *cx, const char *in);

bool cx_eval_args(struct cx *cx,
		  struct cx_vec *toks,
		  struct cx_vec *func_args);

bool cx_eval_rets(struct cx *cx,
		  struct cx_vec *toks,
		  struct cx_vec *args,
		  struct cx_vec *rets);

#endif
