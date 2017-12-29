#ifndef CX_EVAL_H
#define CX_EVAL_H

#include <stdbool.h>

struct cx;
struct cx_vec;


bool cx_eval(struct cx *cx, struct cx_vec *toks, struct cx_tok *pc);
bool cx_eval_str(struct cx *cx, const char *in);

bool cx_scan_args(struct cx *cx, struct cx_func *func);

bool cx_eval_args(struct cx *cx,
		  struct cx_vec *toks,
		  struct cx_vec *ids,
		  struct cx_vec *func_args);

#endif
