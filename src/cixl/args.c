#include <stdlib.h>
#include <string.h>

#include "cixl/args.h"
#include "cixl/type.h"

struct cx_arg *cx_arg_deinit(struct cx_arg *arg) {
  if (arg->id) { free(arg->id); }
  if (!arg->type && arg->narg == -1) { cx_box_deinit(&arg->value); }
  return arg;
}

struct cx_arg cx_arg(const char *id, struct cx_type *type) {
  return (struct cx_arg) { .id = id ? strdup(id) : NULL, .type = type };
}

struct cx_arg cx_varg(struct cx_box *value) {
  struct cx_arg arg = { .id = NULL, .type = NULL, .narg = -1};
  cx_copy(&arg.value, value);
  return arg;
}

struct cx_arg cx_narg(const char *id, int n) {
  return (struct cx_arg) { .id = id ? strdup(id) : NULL, .type = NULL, .narg = n };
}

void cx_arg_print(struct cx_arg *a, FILE *out) {
  if (a->type) {
    fputs(a->type->id, out);
  } else if (a->narg != -1) {
    fprintf(out, "%d", a->narg);
  } else {
    cx_dump(&a->value, out);
  }
}
