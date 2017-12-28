#ifndef CX_EVAL_H
#define CX_EVAL_H

#include <stdbool.h>

struct cx;
struct cx_vec;

ssize_t cx_scan_args(struct cx *cx,
		     struct cx_func *func,
		     struct cx_vec *toks,
		     ssize_t pc);

ssize_t cx_eval_tok(struct cx *cx, struct cx_vec *toks, ssize_t pc);
bool cx_eval(struct cx *cx, struct cx_vec *toks, ssize_t pc);
bool cx_eval_str(struct cx *cx, const char *in);

bool cx_eval_args(struct cx *cx,
		  struct cx_vec *toks,
		  struct cx_vec *ids,
		  struct cx_vec *func_args);

#endif
