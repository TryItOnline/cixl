#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/malloc.h"
#include "cixl/pair.h"

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

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_pair = cx_pair_ref(src->as_pair);
}

static void clone_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_pair = cx_pair_new(src->type->lib->cx, NULL, NULL);
  cx_clone(&dst->as_pair->x, &src->as_pair->x);
  cx_clone(&dst->as_pair->y, &src->as_pair->y);
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
  fputc(' ', out);
  cx_dump(&v->as_pair->y, out);
  fprintf(out, ")r%d", v->as_pair->nrefs);
}

static void print_imp(struct cx_box *v, FILE *out) {
  cx_print(&v->as_pair->x, out);
  cx_print(&v->as_pair->y, out);
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
