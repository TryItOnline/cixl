#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/malloc.h"
#include "cixl/pair.h"

struct cx_pair *cx_pair_new(struct cx *cx, struct cx_box *a, struct cx_box *b) {
  struct cx_pair *pair = cx_malloc(&cx->pair_alloc);
  if (a) { cx_copy(&pair->a, a); }
  if (b) { cx_copy(&pair->b, b); }
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
    cx_box_deinit(&pair->a);
    cx_box_deinit(&pair->b);
    cx_free(&cx->pair_alloc, pair);
  }
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return
    cx_equid(&x->as_pair->a, &y->as_pair->a) &&
    cx_equid(&x->as_pair->b, &y->as_pair->b);
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  return
    cx_eqval(&x->as_pair->a, &y->as_pair->a) &&
    cx_eqval(&x->as_pair->b, &y->as_pair->b);
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  enum cx_cmp res = cx_cmp(&x->as_pair->a, &y->as_pair->a);
  if (res == CX_CMP_EQ) { res = cx_cmp(&x->as_pair->b, &y->as_pair->b); }
  return res;
}

static bool ok_imp(struct cx_box *v) {
  return cx_ok(&v->as_pair->a) && cx_ok(&v->as_pair->b);
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_pair = cx_pair_ref(src->as_pair);
}

static void clone_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_pair = cx_pair_new(src->type->lib->cx, NULL, NULL);
  cx_clone(&dst->as_pair->a, &src->as_pair->a);
  cx_clone(&dst->as_pair->b, &src->as_pair->b);
}

static void write_imp(struct cx_box *v, FILE *out) {
  fputc('(', out);
  cx_write(&v->as_pair->a, out);
  fputc(' ', out);
  cx_write(&v->as_pair->b, out);
  fputs(",)", out);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  cx_dump(&v->as_pair->a, out);
  fputc(' ', out);
  cx_dump(&v->as_pair->b, out);
  fputc(',', out);
}

static void print_imp(struct cx_box *v, FILE *out) {
  cx_print(&v->as_pair->a, out);
  cx_print(&v->as_pair->b, out);
}

static void deinit_imp(struct cx_box *v) {
  cx_pair_deref(v->as_pair, v->type->lib->cx);
}

struct cx_type *cx_init_pair_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Pair", cx->cmp_type);
  cx_type_push_args(t, cx->opt_type, cx->opt_type);

  t->eqval = eqval_imp;
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->clone = clone_imp;
  t->copy = copy_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->print = print_imp;
  t->deinit = deinit_imp;
  return t;
}
