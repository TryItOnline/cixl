#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/nil.h"
#include "cixl/scope.h"

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return true;
}

static bool ok_imp(struct cx_box *x) {
  return false;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fputs("#nil", out);
}

struct cx_type *cx_init_nil_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Nil", cx->opt_type);
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->write = dump_imp;
  t->dump = dump_imp;

  cx_box_init(cx_set_const(cx, cx_sym(cx, "nil"), false), t);

  return t;
}
