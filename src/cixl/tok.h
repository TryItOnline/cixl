#ifndef CX_TOK_H
#define CX_TOK_H

#include "cixl/box.h"
#include "cixl/vec.h"

#define cx_tok_type(id, ...)			\
  struct cx_tok_type *id() {			\
    static struct cx_tok_type type;		\
    static bool init = true;			\
						\
    if (init) {					\
      cx_tok_type_init(&type);			\
      __VA_ARGS__;				\
    }						\
						\
    return &type;				\
  }						\

struct cx_bin;
struct cx_tok;

struct cx_tok_type {
  ssize_t (*compile)(size_t tok_idx, struct cx_bin *bin, struct cx *cx);
  void (*copy)(struct cx_tok *dst, struct cx_tok *src);
  void (*deinit)(struct cx_tok *);
};

struct cx_tok_type *cx_tok_type_init(struct cx_tok_type *type);

struct cx_tok {
  struct cx_tok_type *type;
  int row, col;

  union {
    struct cx_box as_box;
    struct cx_vec as_vec;
    void *as_ptr;
  };
};

struct cx_tok *cx_tok_init(struct cx_tok *tok,
			   struct cx_tok_type *type,
			   int row, int col);

struct cx_tok *cx_tok_deinit(struct cx_tok *tok);

void cx_tok_copy(struct cx_tok *dst, struct cx_tok *src);

struct cx_tok_type *cx_cut_tok();
struct cx_tok_type *cx_end_tok();
struct cx_tok_type *cx_func_tok();
struct cx_tok_type *cx_group_tok();
struct cx_tok_type *cx_id_tok();
struct cx_tok_type *cx_lambda_tok();
struct cx_tok_type *cx_literal_tok();
struct cx_tok_type *cx_macro_tok();
struct cx_tok_type *cx_type_tok();
struct cx_tok_type *cx_ungroup_tok();
struct cx_tok_type *cx_unlambda_tok();

#endif
