#include "cixl/bool.h"
#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/scope.h"

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_bool == y->as_bool;
}

static bool ok_imp(struct cx_box *v) {
  return v->as_bool;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fputs(v->as_bool ? "#t" : "#f", out);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  fprintf(out,
	  "cx_box_init(%s, cx->bool_type)->as_bool = %s;",
	  exp, v->as_bool ? "true" : "false");
  return true;
}

struct cx_type *cx_init_bool_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Bool", cx->any_type);
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->write = dump_imp;
  t->dump = dump_imp;
  t->emit = emit_imp;
  
  cx_box_init(cx_set_const(cx, cx_sym(cx, "t"), false), t)->as_bool = true;
  cx_box_init(cx_set_const(cx, cx_sym(cx, "f"), false), t)->as_bool = false;

  return t;
}
