#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/bool.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_bool == y->as_bool;
}

static bool ok_imp(struct cx_box *v) {
  return v->as_bool;
}

static void fprint_imp(struct cx_box *v, FILE *out) {
  fputs(v->as_bool ? "#t" : "#f", out);
}

struct cx_type *cx_init_bool_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Bool", cx->any_type);
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->fprint = fprint_imp;

  cx_box_init(cx_set_const(cx, "t", false), t)->as_bool = true;
  cx_box_init(cx_set_const(cx, "f", false), t)->as_bool = false;
  
  return t;
}
