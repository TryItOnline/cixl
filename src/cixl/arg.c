#include <inttypes.h>
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

static struct cx_type *ref_type_new_imp(struct cx_type *t,
					const char *id,
					int nargs, struct cx_type *args[]) {
  return &cx_arg_ref_new(t->lib->cx, id)->imp;
}

static void *ref_type_deinit_imp(struct cx_type *t) {
  struct cx_arg_ref *r = cx_baseof(t, struct cx_arg_ref, imp);
  cx_vec_deinit(&r->indices);
  return r;
}

struct cx_arg_ref *cx_arg_ref_new(struct cx *cx, const char *id) {
  return cx_arg_ref_init(malloc(sizeof(struct cx_arg_ref)), cx, id);
}

struct cx_arg_ref *cx_arg_ref_init(struct cx_arg_ref *r,
				   struct cx *cx,
				   const char *id) {
  cx_type_init(&r->imp, cx->lobby, id);
  r->imp.meta = CX_TYPE_ARG;
  cx_derive(&r->imp, cx->any_type);
  r->imp.type_new = ref_type_new_imp;
  r->imp.type_deinit = ref_type_deinit_imp;
  cx_vec_init(&r->indices, sizeof(int));
  char *ip = (char *)id+3;
  
  do {
    *(int *)cx_vec_push(&r->indices) = strtoimax(ip, &ip, 10);
  } while (*ip++ == ':');
  
  return r;
}

struct cx_arg_ref *cx_arg_ref_deinit(struct cx_arg_ref *r) {
  cx_type_deinit(&r->imp);
  return r;
}

struct cx_arg_ref *cx_add_arg_ref(struct cx *cx, const char *id) {
  struct cx_arg_ref *r = cx_arg_ref_new(cx, id);
  *(struct cx_type **)cx_vec_push(&cx->types) = &r->imp;
  *(struct cx_type **)cx_test(cx_set_insert(&cx->lobby->types, &id)) = &r->imp;
  return r;
}

struct cx_type *cx_resolve_arg_refs(struct cx_type *pt,
				    struct cx_type *t,
				    struct cx_type *(*get)(int)) {
  struct cx *cx = t->lib->cx;
  struct cx_type *tt = t;
  
  if (t->meta == CX_TYPE_ARG) {
    struct cx_arg_ref *ar = cx_baseof(t, struct cx_arg_ref, imp);
    int *i = cx_vec_start(&ar->indices);
    tt = get(*i);

    if (!tt) {
      cx_error(cx, cx->row, cx->col, "Failed resolving type arg");
      return NULL;
    }

    if (i+1 != cx_vec_end(&ar->indices)) {
      tt = cx_type_arg(tt, *(struct cx_type **)cx_vec_get(&pt->args, *i), *(i+1));
      i += 2;
		       
      while (i != cx_vec_end(&ar->indices)) {
	if (*i >= tt->args.count) {
	  cx_error(cx, cx->row, cx->col, "Arg index out of bounds: %d", *i);
	  return NULL;
	}
	
	tt = cx_resolve_arg_refs(tt,
				 *(struct cx_type **)cx_vec_get(&tt->args, *i),
				 get);
	i++;
      }
    }
  } else {
    cx_do_vec(&t->args, struct cx_type *, at) {
      struct cx_type *art = cx_resolve_arg_refs(pt, *at, get);

      if (art != *at) {
	struct cx_vec args;
	cx_vec_init(&args, sizeof(struct cx_type *));
	
	for (struct cx_type **_at = cx_vec_start(&t->args);
	     _at != cx_vec_end(&t->args);
	     _at++) {
	  *(struct cx_type **)cx_vec_push(&args) = (_at == at)
	    ? art
	    : cx_resolve_arg_refs(pt, *_at, get);
	}
	
	tt = cx_type_vget(t->raw, args.count, (struct cx_type **)args.items);
	cx_vec_deinit(&args);
	break;
      }
    }
  }

  return tt;
}

