#include <stdbool.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/op.h"
#include "cixl/tok.h"
#include "cixl/scope.h"
#include "cixl/types/bin.h"

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_bin *prev_bin = cx->bin;
  struct cx_op *prev_op = cx->op;

  cx->bin = value->as_ptr;
  cx->op = cx_vec_start(&cx->bin->ops);
  bool ok = false;
  
  while (cx->op != cx_vec_end(&cx->bin->ops)) {
    if (!cx_eval_next(cx)) { goto exit; }
  }

  ok = true;
 exit:
  cx->bin = prev_bin;
  cx->op = prev_op;
  return ok;
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  struct cx_bin *b = src->as_ptr;
  dst->as_ptr = b;
  b->nrefs++;
}

static void fprint_imp(struct cx_box *value, FILE *out) {
  struct cx_bin *b = value->as_ptr;
  fprintf(out, "Bin(%p)@%d", b, b->nrefs);
}

static void deinit_imp(struct cx_box *value) {
  struct cx_bin *b = value->as_ptr;
  cx_test(b->nrefs > 0);
  b->nrefs--;
  if (!b->nrefs) { free(cx_bin_deinit(b)); }
}

struct cx_type *cx_init_bin_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Bin", cx->any_type, NULL);
  t->equid = equid_imp;
  t->call = call_imp;
  t->copy = copy_imp;
  t->fprint = fprint_imp;
  t->deinit = deinit_imp;
  return t;
}
