#ifndef CX_EVAL_H
#define CX_EVAL_H

#include <stdbool.h>

struct cx;
struct cx_vec;

struct cx_tok_type *cx_cls_tok();
struct cx_tok_type *cx_cut_tok();
struct cx_tok_type *cx_dup_tok();
struct cx_tok_type *cx_end_tok();
struct cx_tok_type *cx_false_tok();
struct cx_tok_type *cx_func_tok();
struct cx_tok_type *cx_func_imp_tok();
struct cx_tok_type *cx_group_tok();
struct cx_tok_type *cx_id_tok();
struct cx_tok_type *cx_lambda_tok();
struct cx_tok_type *cx_literal_tok();
struct cx_tok_type *cx_macro_tok();
struct cx_tok_type *cx_nil_tok();
struct cx_tok_type *cx_true_tok();
struct cx_tok_type *cx_type_tok();
struct cx_tok_type *cx_ungroup_tok();
struct cx_tok_type *cx_unlambda_tok();
struct cx_tok_type *cx_zap_tok();

bool cx_eval(struct cx *cx, struct cx_vec *toks, struct cx_tok *pc);
bool cx_eval_str(struct cx *cx, const char *in);

bool cx_scan_args(struct cx *cx, struct cx_func *func);

bool cx_eval_args(struct cx *cx,
		  struct cx_vec *toks,
		  struct cx_vec *ids,
		  struct cx_vec *func_args);

#endif
