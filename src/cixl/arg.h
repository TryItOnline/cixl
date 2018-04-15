#ifndef CX_ARG_H
#define CX_ARG_H

#include <stdio.h>

#include "cixl/box.h"
#include "cixl/sym.h"
#include "cixl/type.h"
#include "cixl/vec.h"

#define cx_args(...)				\
  sizeof((struct cx_arg[]){__VA_ARGS__}) /	\
  sizeof(struct cx_arg),			\
    (struct cx_arg[]){__VA_ARGS__}		\

struct cx_type;
struct cx_vec;

enum cx_arg_type { CX_ARG, CX_NARG, CX_VARG };

struct cx_narg {
  int i, j;
};

struct cx_arg {
  char *id;
  struct cx_sym sym_id;
  enum cx_arg_type arg_type;
  
  union {
    struct cx_narg as_narg;
    struct cx_type *type;
    struct cx_box value;
  };
};

struct cx_arg *cx_arg_deinit(struct cx_arg *arg);
struct cx_arg cx_arg(const char *id, struct cx_type *type);
struct cx_arg cx_varg(struct cx_box *value);
struct cx_arg cx_narg(const char *id, int i, int j);
void cx_arg_print(struct cx_arg *a, FILE *out);
void cx_arg_emit(struct cx_arg *a, FILE *out, struct cx *cx);

bool cx_parse_args(struct cx *cx, struct cx_vec *toks, struct cx_vec *args);

struct cx_arg_ref {
  struct cx_type imp;
  struct cx_vec indices;
};

struct cx_arg_ref *cx_arg_ref_new(struct cx *cx, const char *id);

struct cx_arg_ref *cx_arg_ref_init(struct cx_arg_ref *r,
				   struct cx *cx,
				   const char *id);

struct cx_arg_ref *cx_arg_ref_deinit(struct cx_arg_ref *r);

struct cx_arg_ref *cx_add_arg_ref(struct cx *cx, const char *id);

struct cx_type *cx_resolve_arg_refs(struct cx_type *pt,
				    struct cx_type *t,
				    struct cx_type *(*get)(int));

#endif
