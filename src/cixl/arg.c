#include <stdlib.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/type.h"

struct cx_arg *cx_arg_deinit(struct cx_arg *a) {
  if (a->id) { free(a->id); }
  if (a->arg_type == CX_VARG) { cx_box_deinit(&a->value); }
  return a;
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
    fprintf(out, "Arg%d", a->narg);
    break;
  case CX_VARG:
    cx_dump(&a->value, out);
    break;
  }
}

void cx_arg_emit(struct cx_arg *a, FILE *out) {
    switch (a->arg_type) {
    case CX_ARG:
      fputs("cx_arg(", out);

      if (a->id) {
	fprintf(out, "\"%s\"", a->id);
      } else {
	fputs("NULL", out);
      }

      fprintf(out, ", cx_get_type(*cx->lib, \"%s\", false))", a->type->id);
      break;
    case CX_NARG:
      fputs("cx_narg(", out);

      if (a->id) {
	fprintf(out, "\"%s\"", a->id);
      } else {
	fputs("NULL", out);
      }

      fprintf(out, ", %d)", a->narg);
      break;
    case CX_VARG:
      fputs("({\n"
	    "  struct cx_box v;\n",
	    out);
      
      cx_box_emit(&a->value, "&v", out);
      
      fputs("  cx_varg(&v);\n"
	    "})",
	    out);
      break;
    }
}
