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
#include "cixl/lib/pair.h"

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

static bool zip_imp(struct cx_scope *scope) {
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

static bool unzip_imp(struct cx_scope *scope) {
  struct cx_box *p = cx_test(cx_peek(scope, false)), x, y;
  cx_copy(&x, &p->as_pair->x);
  cx_copy(&y, &p->as_pair->y);
  *cx_box_deinit(p) = x;
  *cx_push(scope) = y;
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

static bool rezip_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  struct cx_box tmp = p.as_pair->x;
  p.as_pair->x = p.as_pair->y;
  p.as_pair->y = tmp;
  cx_box_deinit(&p);
  return true;
}

cx_lib(cx_init_pair, "cx/pair", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");
    cx_use(cx, "cx/iter");
    cx_use(cx, "cx/pair/types");

    cx_add_cfunc(lib, ".", 
		 cx_args(cx_arg("x", cx->opt_type), cx_arg("y", cx->opt_type)),
		 cx_args(cx_arg(NULL, cx->pair_type)),
		 zip_imp);

    cx_add_cfunc(lib, "unzip", 
		 cx_args(cx_arg("p", cx->pair_type)),
		 cx_args(cx_arg(NULL, cx->opt_type), cx_arg(NULL, cx->opt_type)),
		 unzip_imp);

    cx_add_cfunc(lib, "x",
		 cx_args(cx_arg("p", cx->pair_type)),
		 cx_args(cx_arg(NULL, cx->any_type)),
		 x_imp);

    cx_add_cfunc(lib, "y",
		 cx_args(cx_arg("p", cx->pair_type)),
		 cx_args(cx_arg(NULL, cx->any_type)),
		 y_imp);

    cx_add_cfunc(lib, "rezip", 
		 cx_args(cx_arg("p", cx->pair_type)),
		 cx_args(),
		 rezip_imp);  

    cx_add_cxfunc(lib, "rezip", 
		  cx_args(cx_arg("in", cx->seq_type)),
		  cx_args(),
		  "$in &rezip for");
  })

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
  dst->as_pair = cx_pair_new(src->type->cx, NULL, NULL);
  cx_clone(&dst->as_pair->x, &src->as_pair->x);
  cx_clone(&dst->as_pair->y, &src->as_pair->y);
}

static void write_imp(struct cx_box *v, FILE *out) {
  cx_write(&v->as_pair->x, out);
  fputc(' ', out);
  cx_write(&v->as_pair->y, out);
  fputs(".", out);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fputc('(', out);
  cx_dump(&v->as_pair->x, out);
  fputc(' ', out);
  cx_dump(&v->as_pair->y, out);
  fprintf(out, ")r%d", v->as_pair->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_pair_deref(v->as_pair, v->type->cx);
}

cx_lib(cx_init_pair_types, "cx/pair/types", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");

    struct cx_type *t = cx_add_type(lib, "Pair", cx->cmp_type);
    t->eqval = eqval_imp;
    t->equid = equid_imp;
    t->cmp = cmp_imp;
    t->ok = ok_imp;
    t->clone = clone_imp;
    t->copy = copy_imp;
    t->write = write_imp;
    t->dump = dump_imp;
    t->deinit = deinit_imp;
    cx->pair_type = t;
  })
