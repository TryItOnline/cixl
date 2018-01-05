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
      init = false;				\
      cx_tok_type_init(&type);			\
      __VA_ARGS__;				\
    }						\
						\
    return &type;				\
  }						\

struct cx_bin;
struct cx_tok;

struct cx_tok_type {
  ssize_t (*compile)(struct cx_bin *bin, size_t tok_idx, struct cx *cx);
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

struct cx_tok_type *CX_TCUT();
struct cx_tok_type *CX_TEND();
struct cx_tok_type *CX_TFIMP();
struct cx_tok_type *CX_TFUNC();
struct cx_tok_type *CX_TGROUP();
struct cx_tok_type *CX_TID();
struct cx_tok_type *CX_TLAMBDA();
struct cx_tok_type *CX_TLITERAL();
struct cx_tok_type *CX_TMACRO();
struct cx_tok_type *CX_TSTASH();
struct cx_tok_type *CX_TTYPE();
struct cx_tok_type *CX_TUNGROUP();
struct cx_tok_type *CX_TUNLAMBDA();
struct cx_tok_type *CX_TUNTYPE();
struct cx_tok_type *CX_TUNVECT();
struct cx_tok_type *CX_TVECT();
struct cx_tok_type *CX_TZAP();

#endif
