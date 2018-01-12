#include <inttypes.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/int.h"
#include "cixl/types/str.h"
#include "cixl/util.h"

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

static bool for_imp(struct cx_scope *scope) {
  struct cx_box
    act = *cx_test(cx_pop(scope, false)),
    reps = *cx_test(cx_pop(scope, false));

  bool ok = false;
  
  for (int64_t i = 0; i < reps.as_int; i++) {
    cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = i;
    if (!cx_call(&act, scope)) { goto exit; }
  }

 exit:
  cx_box_deinit(&act);
  return ok;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_int == y->as_int;
}

static enum cx_cmp cmp_imp(struct cx_box *x, struct cx_box *y) {
  return cx_cmp_int(&x->as_int, &y->as_int);
}

static bool ok_imp(struct cx_box *v) {
  return v->as_int != 0;
}

static void print_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "%" PRId64, v->as_int);
}

struct cx_type *cx_init_int_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Int", cx->num_type);
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->write = print_imp;
  t->print = print_imp;
  
  cx_add_func(cx, "++", cx_arg(t))->ptr = inc_imp;
  cx_add_func(cx, "--", cx_arg(t))->ptr = dec_imp;
  
  cx_add_func(cx, "char", cx_arg(t))->ptr = char_imp;
  cx_add_func(cx, "str", cx_arg(t))->ptr = str_imp;
  cx_add_func(cx, "times", cx_arg(t), cx_arg(cx->any_type))->ptr = times_imp;
  cx_add_func(cx, "for", cx_arg(t), cx_arg(cx->any_type))->ptr = for_imp;
  
  return t;
}
