#include <inttypes.h>

#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/iter.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/int.h"
#include "cixl/util.h"

struct cx_int_iter {
  struct cx_iter iter;
  int64_t i, end;
};

bool int_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  struct cx_int_iter *it = cx_baseof(iter, struct cx_int_iter, iter);
  
  if (it->i < it->end) {
    cx_box_init(out, scope->cx->int_type)->as_int = it->i;
    it->i++;
    return true;
  }

  iter->done = true;
  return false;
}

void *int_deinit(struct cx_iter *iter) {
  return cx_baseof(iter, struct cx_int_iter, iter);
}

cx_iter_type(int_iter, {
    type.next = int_next;
    type.deinit = int_deinit;
  });

struct cx_int_iter *cx_int_iter_new(int64_t end) {
  struct cx_int_iter *it = malloc(sizeof(struct cx_int_iter));
  cx_iter_init(&it->iter, int_iter());
  it->i = 0;
  it->end = end;
  return it;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_int == y->as_int;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  return cx_cmp_int(&x->as_int, &y->as_int);
}

static bool ok_imp(struct cx_box *v) {
  return v->as_int != 0;
}

static void iter_imp(struct cx_box *in, struct cx_box *out) {
  struct cx *cx = in->type->lib->cx;
  
  cx_box_init(out, cx_type_get(cx->iter_type, cx->int_type))->as_iter =
    &cx_int_iter_new(in->as_int)->iter;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "%" PRId64, v->as_int);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  fprintf(out,
	  "cx_box_init(%s, cx->int_type)->as_int = %" PRId64 ";\n",
	  exp, v->as_int);
  return true;
}

struct cx_type *cx_init_int_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Int", cx->num_type);
  cx_derive(t, cx_type_get(cx->seq_type, t));
  
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->iter = iter_imp;
  t->write = dump_imp;
  t->dump = dump_imp;
  t->emit = emit_imp;  
  return t;
}
