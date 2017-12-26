#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/str.h"
#include "cixl/scope.h"

static void ok_imp(struct cx_scope *scope) {
  struct cx_box x = *cx_ok(cx_pop(scope, false));
  char *v = x.as_ptr;
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = v[0];
  cx_box_deinit(&x);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  return strcmp(x->as_ptr, y->as_ptr) == 0;
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_ptr = strdup(src->as_ptr);
}

static void fprint_imp(struct cx_box *value, FILE *out) {
  fprintf(out, "'%s'", (char *)value->as_ptr);
}

static void deinit_imp(struct cx_box *value) {
  free(value->as_ptr);
}

struct cx_type *cx_init_str_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Str", cx->any_type, NULL);
  t->eqval = eqval_imp;
  t->equid = equid_imp;
  t->copy = copy_imp;
  t->fprint = fprint_imp;
  t->deinit = deinit_imp;
  
  cx_add_func(cx, "?", cx_arg(t))->ptr = ok_imp;

  return t;
}
