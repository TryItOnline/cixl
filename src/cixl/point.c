#include <inttypes.h>

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

struct cx_point *cx_point_init(struct cx_point *p, cx_float_t x, cx_float_t y) {
  p->x = x;
  p->y = y;
  return p;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  struct cx_point *xp = &x->as_point, *yp = &y->as_point;
  return xp->x == yp->x && xp->y == yp->y;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  const struct cx_point *xp = &x->as_point, *yp = &y->as_point; 
  enum cx_cmp cmp = cx_cmp_float(&xp->x, &yp->x);
  if (cmp != CX_CMP_EQ) { return cmp; }
  return cx_cmp_float(&xp->y, &yp->y);
}

static bool ok_imp(struct cx_box *v) {
  struct cx_point *p = &v->as_point;
  return p->x || p->y;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  struct cx_point *p = &v->as_point;
  fprintf(out, "Point(%lf %lf)", p->x, p->y);
}

static void write_imp(struct cx_box *v, FILE *out) {
  struct cx_point *p = &v->as_point;
  fprintf(out, "(%lf %lf xy)", p->x, p->y);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  struct cx_point *p = &v->as_point;

  fprintf(out,
	  "cx_point_init(&cx_box_init(%s, cx->point_type)->as_point, "
	  "%lf, %lf);\n",
	  exp, p->x, p->y);
  
  return true;
}

struct cx_type *cx_init_point_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Point", cx->any_type);
  
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->emit = emit_imp;  
  return t;
}
