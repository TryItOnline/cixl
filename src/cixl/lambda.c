#include <stdlib.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/call_iter.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/lambda.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

struct cx_lambda *cx_lambda_new(struct cx_scope *scope,
				size_t start_pc,
				size_t nops) {
  struct cx *cx = scope->cx;
  struct cx_lambda *l = cx_malloc(&cx->lambda_alloc);
  l->lib = *cx->lib;
  l->scope = cx_scope_ref(scope);
  l->bin = cx_bin_ref(cx->bin);
  l->start_pc = start_pc;
  l->nops = nops;
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
  struct cx_lambda *l = cx_lambda_ref(value->as_ptr);
  
  bool pop_lib = false;
  
  if (*cx->lib != l->lib) {
    cx_push_lib(cx, l->lib);
    pop_lib = true;
  }

  bool pop_scope = false;
  
  if (scope != l->scope) {
    cx_push_scope(cx, l->scope);
    *(struct cx_scope **)cx_vec_push(&l->scope->parents) = cx_scope_ref(scope);
    pop_scope = true;
  }

  bool ok = cx_eval(l->bin, l->start_pc, l->start_pc+l->nops, cx);

  if (pop_scope) {
    cx_test(cx_pop_scope(cx, false));
    
    if (*(struct cx_scope **)cx_vec_peek(&l->scope->parents, 0) == scope) {
      cx_vec_pop(&l->scope->parents);
    }
    
    cx_scope_deref(scope);
  }
  
  if (pop_lib && *cx->lib == l->lib) { cx_pop_lib(cx); }
  cx_lambda_deref(l);
  return ok;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_ptr = cx_lambda_ref(src->as_ptr);
}

static struct cx_iter *iter_imp(struct cx_box *v) {
  return cx_call_iter_new(v);
}

static void dump_imp(struct cx_box *value, FILE *out) {
  struct cx_lambda *l = value->as_ptr;
  fprintf(out, "Lambda(%p)r%d", l, l->nrefs);
}

static void deinit_imp(struct cx_box *value) {
  cx_lambda_deref(value->as_ptr);
}

struct cx_type *cx_init_lambda_type(struct cx_lib *lib) {
  struct cx_type *t = cx_add_type(lib, "Lambda", lib->cx->seq_type);
  t->equid = equid_imp;
  t->call = call_imp;
  t->copy = copy_imp;
  t->iter = iter_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
