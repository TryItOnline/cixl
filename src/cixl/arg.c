#include <stdlib.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/cx.h"
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
    .type = type
  };
}

struct cx_arg cx_varg(struct cx_box *value) {
  struct cx_arg arg = { .arg_type = CX_VARG, .id = NULL };
  cx_copy(&arg.value, value);
  return arg;
}

struct cx_arg cx_narg(const char *id, int i, int j) {
  return (struct cx_arg) {
    .arg_type = CX_NARG,
    .id = id ? strdup(id) : NULL,
    .as_narg = (struct cx_narg) {.i=i, .j=j}
  };
}

void cx_arg_print(struct cx_arg *a, FILE *out) {
  switch(a->arg_type) {
  case CX_ARG:
    fputs(a->type->id, out);
    break;
  case CX_NARG:
    fprintf(out, "Arg%d:%d", a->as_narg.i, a->as_narg.j);
    break;
  case CX_VARG:
    cx_dump(&a->value, out);
    break;
  }
}

void cx_arg_emit(struct cx_arg *a, FILE *out, struct cx *cx) {
    switch (a->arg_type) {
    case CX_ARG:
      fputs("cx_arg(", out);

      if (a->id) {
	fprintf(out, "\"%s\"", a->id);
      } else {
	fputs("NULL", out);
      }

      fprintf(out, ", cx_get_type(cx, \"%s\", false))", a->type->id);
      break;
    case CX_NARG:
      fputs("cx_narg(", out);

      if (a->id) {
	fprintf(out, "\"%s\"", a->id);
      } else {
	fputs("NULL", out);
      }

      fprintf(out, ", %d, %d)", a->as_narg.i, a->as_narg.j);
      break;
    case CX_VARG: {
      struct
	cx_sym v_var = cx_gsym(cx, "v"),
	vp_var = cx_gsym(cx, "vp"),
	a_var = cx_gsym(cx, "a");
      
      fprintf(out,
	      "({\n"
	      "  struct cx_box %s, *%s=&%s;\n",
	      v_var.id, vp_var.id, v_var.id);
      
      cx_box_emit(&a->value, vp_var.id, out);
      
      fprintf(out,
	      "  struct cx_arg %s = cx_varg(%s);\n"
	      "  cx_box_deinit(%s);\n"
	      "  %s;\n"
	      "})\n",
	      a_var.id, vp_var.id, vp_var.id, a_var.id);
      break;
    }}
}
