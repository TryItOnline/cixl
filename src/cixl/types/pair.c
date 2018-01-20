#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/iter.h"
#include "cixl/types/pair.h"

struct cx_pair *cx_pair_new(struct cx *cx, struct cx_box *x, struct cx_box *y) {
  struct cx_pair *pair = cx_malloc(&cx->pair_alloc);
  if (x) { cx_copy(&pair->x, x); }
  if (y) { cx_copy(&pair->y, y); }
  pair->nrefs = 1;
  return pair;
}

struct cx_pair *cx_pair_ref(struct cx_pair *pair) {
  pair->nrefs++;
  return pair;
}

void cx_pair_deref(struct cx_pair *pair, struct cx *cx) {
  cx_test(pair->nrefs);
  pair->nrefs--;
  
  if (!pair->nrefs) {
    cx_box_deinit(&pair->x);
    cx_box_deinit(&pair->y);
    cx_free(&cx->pair_alloc, pair);
  }
}

static bool cons_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  struct cx_pair *p = cx_pair_new(cx, NULL, NULL);
  p->x = x;
  p->y = y;
  
  cx_box_init(cx_push(scope), cx->pair_type)->as_pair = p;
  return true;
}

static bool x_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  cx_copy(cx_push(scope), &p.as_pair->x);
  cx_box_deinit(&p);
  return true;
}

static bool y_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  cx_copy(cx_push(scope), &p.as_pair->y);
  cx_box_deinit(&p);
  return true;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return
    cx_equid(&x->as_pair->x, &y->as_pair->x) &&
    cx_equid(&x->as_pair->y, &y->as_pair->y);
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  return
    cx_eqval(&x->as_pair->x, &y->as_pair->x) &&
    cx_eqval(&x->as_pair->y, &y->as_pair->y);
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  enum cx_cmp res = cx_cmp(&x->as_pair->x, &y->as_pair->x);
  if (res == CX_CMP_EQ) { res = cx_cmp(&x->as_pair->y, &y->as_pair->y); }
  return res;
}

static bool ok_imp(struct cx_box *v) {
  return cx_ok(&v->as_pair->x) && cx_ok(&v->as_pair->y);
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_pair = cx_pair_ref(src->as_pair);
}

static void write_imp(struct cx_box *v, FILE *out) {
  fputc('(', out);
  cx_write(&v->as_pair->x, out);
  fputc(' ', out);
  cx_write(&v->as_pair->y, out);
  fputs(" .)", out);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fputc('(', out);
  cx_dump(&v->as_pair->x, out);
  fputc('.', out);
  cx_dump(&v->as_pair->y, out);
  fprintf(out, ")@%d", v->as_pair->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_pair_deref(v->as_pair, v->type->cx);
}

struct cx_type *cx_init_pair_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Pair", cx->any_type, cx->cmp_type);
  t->eqval = eqval_imp;
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;

  cx_add_cfunc(cx, ".", cons_imp, 
	       cx_arg("x", cx->opt_type), cx_arg("y", cx->opt_type));
  
  cx_add_cfunc(cx, "x", x_imp, cx_arg("p", t));
  cx_add_cfunc(cx, "y", y_imp, cx_arg("p", t));

  return t;
}
