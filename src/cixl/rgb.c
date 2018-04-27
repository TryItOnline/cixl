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

struct cx_rgb *cx_rgb_init(struct cx_rgb *c,
			   int r, int g, int b) {
  c->r = r;
  c->g = g;
  c->b = b;
  return c;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  struct cx_rgb *xv = &x->as_rgb, *yv = &y->as_rgb;
  return xv->r == yv->r && xv->g == yv->g && xv->b == yv->b;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  const struct cx_rgb *xv = &x->as_rgb, *yv = &y->as_rgb;
  
  enum cx_cmp cmp = cx_cmp_cint(&xv->r, &yv->r);
  if (cmp != CX_CMP_EQ) { return cmp; }
  cmp = cx_cmp_cint(&xv->g, &yv->g);
  if (cmp != CX_CMP_EQ) { return cmp; }
  return cx_cmp_cint(&xv->b, &yv->b);
}

static bool ok_imp(struct cx_box *v) {
  struct cx_rgb *vv = &v->as_rgb;
  return vv->r || vv->g || vv->b;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  struct cx_rgb *vv = &v->as_rgb;
  fprintf(out, "RGB(%d %d %d)", vv->r, vv->g, vv->b);
}

static void write_imp(struct cx_box *v, FILE *out) {
  struct cx_rgb *vv = &v->as_rgb;
  fprintf(out, "(%d %d %d new-rgb)", vv->r, vv->g, vv->b);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  struct cx_rgb *vv = &v->as_rgb;

  fprintf(out,
	  "cx_rgb_init(&cx_box_init(%s, cx->rgb_type)->as_rgb, %d, %d, %d);\n",
	  exp, vv->r, vv->g, vv->b);
  
  return true;
}

struct cx_type *cx_init_rgb_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "RGB", cx->color_type);
  
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->emit = emit_imp;  
  return t;
}
