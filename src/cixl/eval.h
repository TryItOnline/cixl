#ifndef CX_EVAL_H
#define CX_EVAL_H

#include <stdbool.h>

struct cx;
struct cx_bin;
struct cx_func;
struct cx_op;
struct cx_vec;

bool cx_eval_next(struct cx *cx);
bool cx_eval(struct cx *cx, struct cx_bin *bin, struct cx_op *start);
bool cx_eval_str(struct cx *cx, const char *in);

bool cx_scan_args(struct cx *cx, struct cx_func *func);

bool cx_eval_args(struct cx *cx,
		  struct cx_vec *toks,
		  struct cx_vec *ids,
		  struct cx_vec *func_args);

#endif
