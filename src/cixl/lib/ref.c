#include <string.h>
#include <inttypes.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/scope.h"
#include "cixl/lib.h"
#include "cixl/lib/ref.h"

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

static bool ref_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box v = *cx_test(cx_pop(scope, false));
  struct cx_ref *r = cx_ref_new(cx, NULL);
  r->value = v;
  cx_box_init(cx_push(scope), cx->ref_type)->as_ref = r;
  return true;
}

static bool get_imp(struct cx_scope *scope) {
  struct cx_box r = *cx_test(cx_pop(scope, false));
  cx_copy(cx_push(scope), &r.as_ref->value);
  cx_box_deinit(&r);
  return true;
}

static bool put_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    r = *cx_test(cx_pop(scope, false));

  cx_box_deinit(&r.as_ref->value);
  r.as_ref->value = v;
  cx_box_deinit(&r);
  return true;
}

cx_lib(cx_init_ref, "cx/ref", { 
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");
    cx_use(cx, "cx/ref/types");

    cx_add_cfunc(lib, "ref",
		 cx_args(cx_arg("val", cx->opt_type)),
		 cx_args(cx_arg(NULL, cx->ref_type)),
		 ref_imp);
  
    cx_add_cfunc(lib, "get-ref",
		 cx_args(cx_arg("ref", cx->ref_type)),
		 cx_args(cx_arg(NULL, cx->opt_type)),
		 get_imp);

    cx_add_cfunc(lib, "put-ref",
		 cx_args(cx_arg("ref", cx->ref_type), cx_arg("val", cx->opt_type)),
		 cx_args(),
		 put_imp);
  })

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
  cx_dump(&v->as_ref->value, out);
  fputs(" ref", out);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fputs("Ref(", out);
  cx_dump(&v->as_ref->value, out);
  
  fprintf(out, ")r%d", v->as_ref->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_ref_deref(v->as_ref);
}

cx_lib(cx_init_ref_types, "cx/ref/types", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");
    struct cx_type *t = cx_add_type(lib, "Ref", cx->any_type);
    t->eqval = eqval_imp;
    t->equid = equid_imp;
    t->ok = ok_imp;
    t->copy = copy_imp;
    t->write = write_imp;
    t->dump = dump_imp;
    t->deinit = deinit_imp;
    cx->ref_type = t;
  })
