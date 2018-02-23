#ifndef CX_ARGS_H
#define CX_ARGS_H

#include <stdio.h>

#include "cixl/box.h"
#include "cixl/sym.h"

#define cx_args(...)				\
  sizeof((struct cx_arg[]){__VA_ARGS__}) /	\
  sizeof(struct cx_arg),			\
    (struct cx_arg[]){__VA_ARGS__}		\

struct cx_type;
struct cx_vec;

struct cx_arg {
  char *id;
  struct cx_sym sym_id;
  struct cx_type *type;
  struct cx_box value;
  int narg;
};

struct cx_arg *cx_arg_deinit(struct cx_arg *arg);
struct cx_arg cx_arg(const char *id, struct cx_type *type);
struct cx_arg cx_varg(struct cx_box *value);
struct cx_arg cx_narg(const char *id, int n);
void cx_arg_print(struct cx_arg *a, FILE *out);

bool cx_parse_args(struct cx *cx, struct cx_vec *toks, struct cx_vec *args);

#endif
