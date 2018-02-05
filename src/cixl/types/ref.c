#include <inttypes.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/ref.h"

struct cx_ref *cx_ref_new(struct cx *cx, struct cx_box *value) {
  struct cx_ref *ref = cx_malloc(&cx->ref_alloc);
  if (value) { cx_copy(&ref->value, value); }
  ref->nrefs = 1;
  return ref;
}

struct cx_ref *cx_ref_ref(struct cx_ref *ref) {
  ref->nrefs++;
  return ref;
}

void cx_ref_deref(struct cx_ref *ref) {
  cx_test(ref->nrefs);
  ref->nrefs--;
  if (!ref->nrefs) { cx_free(&ref->value.type->cx->ref_alloc, ref); }
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ref == y->as_ref;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  return cx_eqval(&x->as_ref->value, &y->as_ref->value);
}

static bool ok_imp(struct cx_box *v) {
  return cx_ok(&v->as_ref->value);
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_ref = cx_ref_ref(src->as_ref);
}

static void write_imp(struct cx_box *v, FILE *out) {
  fputc('(', out);
  cx_dump(&v->as_ref->value, out);
  fputs(" ref)", out);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fputs("Ref(", out);
  cx_dump(&v->as_ref->value, out);
  
  fprintf(out, ")r%d", v->as_ref->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_ref_deref(v->as_ref);
}

struct cx_type *cx_init_ref_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Ref", cx->any_type);
  t->eqval = eqval_imp;
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;

  return t;
}
