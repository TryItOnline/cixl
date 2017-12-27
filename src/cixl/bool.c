#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/bool.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/scope.h"

static void not_imp(struct cx_scope *scope) {
  struct cx_box x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = !x.as_bool;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_bool == y->as_bool;
}

static void fprint_imp(struct cx_box *value, FILE *out) {
  fputc(value->as_bool ? 't' : 'f', out);
}

struct cx_type *cx_init_bool_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Bool", cx->any_type, NULL);
  t->equid = equid_imp;
  t->fprint = fprint_imp;

  cx_add_func(cx, "not", cx_arg(t))->ptr = not_imp;

  return t;
}
