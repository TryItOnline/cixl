#include <stdlib.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/scope.h"
#include "cixl/tok.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/vect.h"

static const void *get_imp_arg_tags(const void *value) {
  struct cx_fimp *const *imp = value;
  return &(*imp)->arg_tags;
}

enum cx_cmp cmp_arg_tags(const void *x, const void *y) {
  const cx_type_tag_t *xv = x, *yv = y;
  if (*xv < *yv) { return CX_CMP_LT; }
  return (*xv > *yv) ? CX_CMP_GT : CX_CMP_EQ;
}

struct cx_func *cx_func_init(struct cx_func *func,
			     struct cx *cx,
			     const char *id,
			     int nargs) {
  func->cx = cx;
  func->id = strdup(id);
  cx_set_init(&func->imps, sizeof(struct cx_fimp *), cmp_arg_tags);
  func->imps.key = get_imp_arg_tags;
  func->nargs = nargs;
  return func;
}

struct cx_func *cx_func_deinit(struct cx_func *func) {
  free(func->id);
  cx_do_set(&func->imps, struct cx_fimp *, i) { free(cx_fimp_deinit(*i)); }
  cx_set_deinit(&func->imps);
  return func; 
}

struct cx_func_arg cx_arg(struct cx_type *type) {
  return (struct cx_func_arg) { type, -1 };
}

struct cx_func_arg cx_narg(int n) {
  return (struct cx_func_arg) { NULL, n };
}

struct cx_fimp *cx_func_add_imp(struct cx_func *func,
				int nargs,
				struct cx_func_arg *args) {
  struct cx_vec imp_args;
  cx_vec_init(&imp_args, sizeof(struct cx_func_arg));
  cx_type_tag_t arg_tags = 0;
  
  if (nargs) {
    cx_vec_grow(&imp_args, nargs);

    for (int i=0; i < nargs; i++) {
      struct cx_func_arg a = args[i];
      *(struct cx_func_arg *)cx_vec_push(&imp_args) = a;
      if (a.type) { arg_tags += a.type->tag; }
    }
  }
    
  struct cx_fimp **found = cx_set_get(&func->imps, &arg_tags);
  
  if (found) {
    cx_set_delete(&func->imps, &arg_tags);
    free(cx_fimp_deinit(*found));
  }

  struct cx_fimp *imp = cx_fimp_init(malloc(sizeof(struct cx_fimp)),
				     func,
				     arg_tags);
  *(struct cx_fimp **)cx_set_insert(&func->imps, &arg_tags) = imp;
  imp->args = imp_args;
  return imp;
}

struct cx_fimp *cx_func_get_imp(struct cx_func *func, struct cx_vec *stack) {
  cx_do_set(&func->imps, struct cx_fimp *, imp) {
    if (cx_fimp_match(*imp, stack)) { return *imp; }
  }

  return NULL;
}

static bool imps_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_func *f = cx_test(cx_pop(scope, false))->as_ptr;
  struct cx_vect *is = cx_vect_new();

  cx_do_set(&f->imps, struct cx_fimp *, i) {
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
  
  struct cx_fimp *imp = cx_func_get_imp(value->as_ptr, &scope->stack);

  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: '%s'", imp->func->id);
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
