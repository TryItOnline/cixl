#include <inttypes.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/int.h"
#include "cixl/types/iter.h"
#include "cixl/types/str.h"
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

static bool inc_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  v->as_int++;
  return true;
}

static bool dec_imp(struct cx_scope *scope) {
  struct cx_box *v = cx_test(cx_peek(scope, false));
  v->as_int--;
  return true;
}

static bool char_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box v = *cx_test(cx_pop(scope, false));
  
  if (v.as_int < 0 || v.as_int > 255) {
    cx_error(cx, cx->row, cx->col, "Invalid char: %" PRId64, v.as_int);
    return false;
  }
  
  cx_box_init(cx_push(scope), cx->char_type)->as_char = v.as_int;
  return true;
}

static bool str_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  char *s = cx_fmt("%" PRId64, v.as_int);
  cx_box_init(cx_push(scope), scope->cx->str_type)->as_str = cx_str_new(s);
  free(s);
  return true;
}

static bool times_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    reps = *cx_test(cx_pop(scope, false));

  bool ok = false;
  
  for (int64_t i = 0; i < reps.as_int; i++) {
    if (!cx_call(&v, scope)) { goto exit; }
  }

  ok = true;
 exit:
  cx_box_deinit(&v);
  return ok;
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

static struct cx_iter *iter_imp(struct cx_box *v) {
  return &cx_int_iter_new(v->as_int)->iter;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "%" PRId64, v->as_int);
}

struct cx_type *cx_init_int_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Int",
				  cx->any_type, cx->num_type, cx->seq_type);
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->iter = iter_imp;
  t->write = dump_imp;
  t->dump = dump_imp;
  
  cx_add_cfunc(cx, "++", inc_imp, cx_arg("v", t));
  cx_add_cfunc(cx, "--", dec_imp, cx_arg("v", t));
  
  cx_add_cfunc(cx, "char", char_imp, cx_arg("v", t));
  cx_add_cfunc(cx, "str", str_imp, cx_arg("v", t));
  cx_add_cfunc(cx, "times", times_imp, cx_arg("n", t), cx_arg("act", cx->any_type));
  
  return t;
}
