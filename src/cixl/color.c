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

struct cx_color *cx_color_init(struct cx_color *c,
			       unsigned int r, unsigned int g, unsigned int b,
			       unsigned int a) {
  c->r = r;
  c->g = g;
  c->b = b;
  c->a = a;
  return c;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  struct cx_color *xv = &x->as_color, *yv = &y->as_color;
  return xv->r == yv->r && xv->g == yv->g && xv->b == yv->b;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  const struct cx_color *xv = &x->as_color, *yv = &y->as_color;
  
  enum cx_cmp cmp = cx_cmp_cint(&xv->r, &yv->r);
  if (cmp != CX_CMP_EQ) { return cmp; }
  cmp = cx_cmp_cint(&xv->g, &yv->g);
  if (cmp != CX_CMP_EQ) { return cmp; }
  return cx_cmp_cint(&xv->b, &yv->b);
}

static bool ok_imp(struct cx_box *v) {
  struct cx_color *vv = &v->as_color;
  return vv->r || vv->g || vv->b;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  struct cx_color *vv = &v->as_color;
  fprintf(out, "Color(%u %u %u %u)", vv->r, vv->g, vv->b, vv->a);
}

static void write_imp(struct cx_box *v, FILE *out) {
  struct cx_color *vv = &v->as_color;
  fprintf(out, "(%u %u %u %u new-color)", vv->r, vv->g, vv->b, vv->a);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  struct cx_color *vv = &v->as_color;

  fprintf(out,
	  "cx_color_init(&cx_box_init(%s, cx->color_type)->as_color, "
	  "%u, %u, %u, %u);\n",
	  exp, vv->r, vv->g, vv->b, vv->a);
  
  return true;
}

struct cx_type *cx_init_color_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Color", cx->any_type);
  
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->emit = emit_imp;  
  return t;
}
