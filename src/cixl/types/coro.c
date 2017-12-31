#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/func.h"
#include "cixl/scope.h"
#include "cixl/tok.h"
#include "cixl/types/coro.h"

struct cx_coro *cx_coro_init(struct cx_coro *coro,
			     struct cx *cx,
			     struct cx_scope *scope) {
  coro->scope = cx_scope_ref(scope);
  coro->nrefs = 1;
  coro->done = false;
  coro->bin = cx_bin_ref(cx->bin);
  coro->op = cx->op;
  return coro;
}

struct cx_coro *cx_coro_deinit(struct cx_coro *coro) {
  cx_scope_unref(coro->scope);
  cx_bin_unref(coro->bin);
  return coro;
}

static void yield_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  if (cx->coro) {
    cx->coro->op = cx->op;
  } else {
    struct cx_coro *coro = cx_coro_init(malloc(sizeof(struct cx_coro)),
					cx,
					scope);

    cx_box_init(cx_push(scope), cx->coro_type)->as_ptr = coro;
  }
  
  cx->stop = true;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_coro *coro = value->as_ptr;
  
  if (coro->done) {
    cx_error(cx, cx->row, cx->col, "Coro is done");
    return false;
  }

  bool pop_scope = false;
  
  if (scope != coro->scope) {
    cx_push_scope(cx, coro->scope);
    pop_scope = true;
  }
  
  cx->coro = coro;
  bool ok = false;
  
  ok = cx_eval(cx, coro->bin, coro->op);
  coro->op = cx->op;
  if (coro->op == cx_vec_end(&coro->bin->ops)) { coro->done = true; }
  
  cx->coro = NULL;
  if (!ok) { return false; }
  if (pop_scope) { cx_pop_scope(cx, false); }
  return true;
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  struct cx_coro *c = src->as_ptr;
  dst->as_ptr = c;
  c->nrefs++;
}

static void fprint_imp(struct cx_box *value, FILE *out) {
  struct cx_coro *coro = value->as_ptr;
  fprintf(out, "Coro(%p@%d)", coro, coro->nrefs);
}

static void deinit_imp(struct cx_box *value) {
  struct cx_coro *coro = value->as_ptr;
  cx_test(coro->nrefs > 0);
  coro->nrefs--;
  if (!coro->nrefs) { free(cx_coro_deinit(coro)); }
}

struct cx_type *cx_init_coro_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Coro", cx->any_type, NULL);
  t->equid = equid_imp;
  t->call = call_imp;
  t->copy = copy_imp;
  t->fprint = fprint_imp;
  t->deinit = deinit_imp;

  cx_add_func(cx, "yield")->ptr = yield_imp;

  return t;
}
