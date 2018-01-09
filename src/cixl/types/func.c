#include <stdlib.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/buf.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/scope.h"
#include "cixl/tok.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/vect.h"

static const void *get_imp_id(const void *value) {
  struct cx_fimp *const *imp = value;
  return &(*imp)->id;
}

struct cx_func *cx_func_init(struct cx_func *func,
			     struct cx *cx,
			     const char *id,
			     int nargs) {
  func->cx = cx;
  func->id = strdup(id);
  cx_set_init(&func->imp_lookup, sizeof(struct cx_fimp *), cx_cmp_str);
  func->imp_lookup.key = get_imp_id;
  cx_vec_init(&func->imps, sizeof(struct cx_fimp *));
  func->nargs = nargs;
  return func;
}

struct cx_func *cx_func_deinit(struct cx_func *func) {
  free(func->id);
  cx_do_set(&func->imp_lookup, struct cx_fimp *, i) { free(cx_fimp_deinit(*i)); }
  cx_set_deinit(&func->imp_lookup);
  cx_vec_deinit(&func->imps);
  return func; 
}

struct cx_func_arg *cx_func_arg_deinit(struct cx_func_arg *arg) {
  if (!arg->type && arg->narg == -1) { cx_box_deinit(&arg->value); }
  return arg;
}

struct cx_func_arg cx_arg(struct cx_type *type) {
  return (struct cx_func_arg) { .type = type };
}

struct cx_func_arg cx_varg(struct cx_box *value) {
  struct cx_func_arg arg = { .type = NULL, .narg = -1};
  cx_copy(&arg.value, value);
  return arg;
}

struct cx_func_arg cx_narg(int n) {
  return (struct cx_func_arg) { .type = NULL, .narg = n };
}

static void fprint_arg_id(struct cx_func_arg *a,
			  struct cx_vec *args,
			  FILE *out) {
  if (a->type) {
    fputs(a->type->id, out);
  } else if (a->narg != -1) {
    fprint_arg_id(cx_vec_get(args, a->narg), args, out);
  } else {
    cx_fprint(&a->value, out);
  }
}

struct cx_fimp *cx_func_add_imp(struct cx_func *func,
				int nargs,
				struct cx_func_arg *args) {
  struct cx_vec imp_args;
  cx_vec_init(&imp_args, sizeof(struct cx_func_arg));

  struct cx_buf id;
  cx_buf_open(&id);
  
  if (nargs) {
    cx_vec_grow(&imp_args, nargs);

    for (int i=0; i < nargs; i++) {
      struct cx_func_arg a = args[i];
      *(struct cx_func_arg *)cx_vec_push(&imp_args) = a;
      if (i) { fputc(' ', id.stream); }
      fprint_arg_id(&a, &imp_args, id.stream);
    }
  }

  cx_buf_close(&id);
  struct cx_fimp **found = cx_set_get(&func->imp_lookup, &id.data);
  struct cx_fimp *imp = NULL;
  
  if (found) {
    imp = *found;
    size_t i = imp->i;
    cx_fimp_deinit(imp);
    cx_fimp_init(imp, func, id.data, i);
  } else {
    imp = cx_fimp_init(malloc(sizeof(struct cx_fimp)),
		       func,
		       id.data,
		       func->imps.count);
    *(struct cx_fimp **)cx_set_insert(&func->imp_lookup, &id.data) = imp;
    *(struct cx_fimp **)cx_vec_push(&func->imps) = imp;
  }
  
  imp->args = imp_args;
  return imp;
}

struct cx_fimp *cx_func_get_imp(struct cx_func *func,
				struct cx_vec *stack,
				size_t offs,
				struct cx_scope *scope) {
  if (offs >= func->imps.count) { return NULL; }
  
  for (struct cx_fimp **i = cx_vec_peek(&func->imps, offs);
       i >= (struct cx_fimp **)func->imps.items;
       i--) {
    if (cx_fimp_match(*i, stack, scope)) { return *i; }
  }

  return NULL;
}

static bool imps_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_func *f = cx_test(cx_pop(scope, false))->as_ptr;
  struct cx_vect *is = cx_vect_new();

  for (struct cx_fimp **i = cx_vec_peek(&f->imps, 0);
       i >= (struct cx_fimp **)f->imps.items;
       i--) {
    cx_box_init(cx_vec_push(&is->imp), cx->fimp_type)->as_ptr = *i;
  }
  
  cx_box_init(cx_push(scope), scope->cx->vect_type)->as_ptr = is;
  return true;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_func *func = value->as_ptr;
  struct cx_fimp *imp = cx_func_get_imp(func, &scope->stack, 0, scope);

  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: '%s'", func->id);
    return -1;
  }

  return cx_fimp_call(imp, scope);
}

static void fprint_imp(struct cx_box *value, FILE *out) {
  struct cx_func *func = value->as_ptr;
  fprintf(out, "Func(%s)", func->id);
}

struct cx_type *cx_init_func_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Func", cx->any_type);
  t->equid = equid_imp;
  t->call = call_imp;
  t->fprint = fprint_imp;

  cx_add_func(cx, "imps", cx_arg(t))->ptr = imps_imp;

  return t;
}
