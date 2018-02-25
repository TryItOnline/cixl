#include <stdlib.h>
#include <string.h>

#include "cixl/args.h"
#include "cixl/error.h"
#include "cixl/type.h"

struct cx_arg *cx_arg_deinit(struct cx_arg *arg) {
  if (arg->id) { free(arg->id); }
  if (!arg->type && arg->narg == -1) { cx_box_deinit(&arg->value); }
  return arg;
}

struct cx_arg cx_arg(const char *id, struct cx_type *type) {
  return (struct cx_arg) {
    .arg_type = CX_ARG,
      .id = id ? strdup(id) : NULL,
      .type = type };
}

struct cx_arg cx_varg(struct cx_box *value) {
  struct cx_arg arg = { .arg_type = CX_VARG, .id = NULL };
  cx_copy(&arg.value, value);
  return arg;
}

struct cx_arg cx_narg(const char *id, int n) {
  return (struct cx_arg) {
    .arg_type = CX_NARG,
      .id = id ? strdup(id) : NULL,
      .narg = n };
}

void cx_arg_print(struct cx_arg *a, FILE *out) {
  switch(a->arg_type) {
  case CX_ARG:
    fputs(a->type->id, out);
    break;
  case CX_NARG:
    fprintf(out, "T%d", a->narg);
    break;
  case CX_VARG:
    cx_dump(&a->value, out);
    break;
  default:
    cx_test(false);
    break;
  }
}
