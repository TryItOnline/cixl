#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/iter.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/int.h"
#include "cixl/util.h"

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_float == y->as_float;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  if (x->as_float < y->as_float) { return CX_CMP_LT; }
  return (x->as_float > y->as_float) ? CX_CMP_GT : CX_CMP_EQ;
}

static bool ok_imp(struct cx_box *v) {
  return v->as_float != 0;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "%lf", v->as_float);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  fprintf(out,
	  "cx_box_init(%s, cx->float_type)->as_float = %lf;\n",
	  exp, v->as_float);
  return true;
}

struct cx_type *cx_init_float_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Float", cx->num_type);
  
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->write = dump_imp;
  t->dump = dump_imp;
  t->emit = emit_imp;  
  return t;
}
