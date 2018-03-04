#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/iter.h"
#include "cixl/scope.h"

struct cx_iter_type *cx_iter_type_init(struct cx_iter_type *type) {
  type->next = NULL;
  type->deinit = NULL;
  return type;
}

struct cx_iter *cx_iter_init(struct cx_iter *iter, struct cx_iter_type *type) {
  iter->type = type;
  iter->nrefs = 1;
  iter->done = false;
  return iter;
}

struct cx_iter *cx_iter_ref(struct cx_iter *iter) {
  iter->nrefs++;
  return iter;
}

void cx_iter_deref(struct cx_iter *iter) {
  cx_test(iter->nrefs);
  iter->nrefs--;

  if (!iter->nrefs) {
    free(iter->type->deinit(iter));
  }
}

bool cx_iter_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  return !iter->done && cx_test(iter->type->next)(iter, out, scope);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_iter == y->as_iter;
}

static bool ok_imp(struct cx_box *v) {
  return !v->as_iter->done;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_iter = cx_iter_ref(src->as_iter);
}

static struct cx_iter *iter_imp(struct cx_box *v) {
  return cx_iter_ref(v->as_iter);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "Iter(%p)r%d", v->as_iter, v->as_iter->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_iter_deref(v->as_iter);
}

struct cx_type *cx_init_iter_type(struct cx_lib *lib) {
  struct cx_type *t = cx_add_type(lib, "Iter", lib->cx->seq_type);
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->iter = iter_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
