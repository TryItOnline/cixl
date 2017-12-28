#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/scope.h"
#include "cixl/types/bool.h"

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_bool == y->as_bool;
}

static bool ok_imp(struct cx_box *v) {
  return v->as_bool;
}

static void fprint_imp(struct cx_box *v, FILE *out) {
  fputc(v->as_bool ? 't' : 'f', out);
}

struct cx_type *cx_init_bool_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Bool", cx->any_type, NULL);
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->fprint = fprint_imp;

  return t;
}
