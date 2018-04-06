#ifndef CX_ARG_H
#define CX_ARG_H

#include <stdio.h>

#include "cixl/box.h"
#include "cixl/sym.h"

#define cx_args(...)				\
  sizeof((struct cx_arg[]){__VA_ARGS__}) /	\
  sizeof(struct cx_arg),			\
    (struct cx_arg[]){__VA_ARGS__}		\

struct cx_type;
struct cx_vec;

enum cx_arg_type { CX_ARG, CX_NARG, CX_VARG };

struct cx_arg {
  char *id;
  struct cx_sym sym_id;
  enum cx_arg_type arg_type;

  union {
    int narg;
    struct cx_type *type;
    struct cx_box value;
  };
};

struct cx_arg *cx_arg_deinit(struct cx_arg *arg);
struct cx_arg cx_arg(const char *id, struct cx_type *type);
struct cx_arg cx_varg(struct cx_box *value);
struct cx_arg cx_narg(const char *id, int n);
void cx_arg_print(struct cx_arg *a, FILE *out);
void cx_arg_emit(struct cx_arg *a, FILE *out, struct cx *cx);

bool cx_parse_args(struct cx *cx, struct cx_vec *toks, struct cx_vec *args);

#endif
