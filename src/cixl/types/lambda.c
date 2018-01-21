#include <stdlib.h>

#include "cixl/call_iter.h"
#include "cixl/cx.h"
#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/tok.h"
#include "cixl/types/lambda.h"

struct cx_lambda *cx_lambda_new(struct cx_scope *scope,
				size_t start_op,
				size_t num_ops) {
  struct cx *cx = scope->cx;
  struct cx_lambda *l = cx_malloc(&cx->lambda_alloc);
  l->scope = cx_scope_ref(scope);
  l->bin = cx_bin_ref(cx->bin);
  l->start_op = start_op;
  l->num_ops = num_ops;
  l->nrefs = 1;
  return l;
}

struct cx_lambda *cx_lambda_ref(struct cx_lambda *lambda) {
  lambda->nrefs++;
  return lambda;
}

void cx_lambda_deref(struct cx_lambda *lambda) {
  cx_test(lambda->nrefs);
  lambda->nrefs--;
  
  if (!lambda->nrefs) {
    struct cx *cx = lambda->scope->cx;
    cx_bin_deref(lambda->bin);
    cx_scope_deref(lambda->scope);
    cx_free(&cx->lambda_alloc, lambda);
  }
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_lambda *l = value->as_ptr;
  bool pop_scope = false;
  
  if (scope != l->scope) {
    cx_push_scope(cx, l->scope);
    pop_scope = true;
  }

  bool ok = cx_eval(cx, l->bin, cx_vec_get(&l->bin->ops, l->start_op));
  if (pop_scope) { cx_pop_scope(cx, false); }
  return ok;
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_ptr = cx_lambda_ref(src->as_ptr);
}

static struct cx_iter *iter_imp(struct cx_box *v) {
  return cx_call_iter_new(v);
}

static void dump_imp(struct cx_box *value, FILE *out) {
  struct cx_lambda *l = value->as_ptr;
  fprintf(out, "Lambda(%p)@%d", l, l->nrefs);
}

static void deinit_imp(struct cx_box *value) {
  cx_lambda_deref(value->as_ptr);
}

struct cx_type *cx_init_lambda_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Lambda", cx->seq_type);
  t->equid = equid_imp;
  t->call = call_imp;
  t->copy = copy_imp;
  t->iter = iter_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
