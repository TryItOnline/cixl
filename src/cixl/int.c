#include <inttypes.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/int.h"
#include "cixl/scope.h"

static void ok_imp(struct cx_scope *scope) {
  struct cx_box x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = x.as_int != 0;
}

static void lt_imp(struct cx_scope *scope) {
  struct cx_box y = *cx_ok(cx_pop(scope, false)), x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = x.as_int < y.as_int;
}

static void gt_imp(struct cx_scope *scope) {
  struct cx_box y = *cx_ok(cx_pop(scope, false)), x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = x.as_int > y.as_int;
}

static void inc_imp(struct cx_scope *scope) {
  struct cx_box x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = x.as_int+1;
}

static void dec_imp(struct cx_scope *scope) {
  struct cx_box x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = x.as_int-1;
}

static void add_imp(struct cx_scope *scope) {
  struct cx_box y = *cx_ok(cx_pop(scope, false)), x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = x.as_int + y.as_int;
}

static void sub_imp(struct cx_scope *scope) {
  struct cx_box y = *cx_ok(cx_pop(scope, false)), x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = x.as_int - y.as_int;
}

static void mul_imp(struct cx_scope *scope) {
  struct cx_box y = *cx_ok(cx_pop(scope, false)), x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = x.as_int * y.as_int;
}

static void div_imp(struct cx_scope *scope) {
  struct cx_box y = *cx_ok(cx_pop(scope, false)), x = *cx_ok(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = x.as_int / y.as_int;
}

static void times_imp(struct cx_scope *scope) {
  struct cx_box
    x = *cx_ok(cx_pop(scope, false)),
    reps = *cx_ok(cx_pop(scope, false));

  for (cx_int_t i = 0; i < reps.as_int; i++) {
    if (!cx_box_call(&x, scope)) { break; }
  }

  cx_box_deinit(&x);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_int == y->as_int;
}

static void fprint_imp(struct cx_box *value, FILE *out) {
  fprintf(out, "%" PRId64, value->as_int);
}

struct cx_type *cx_init_int_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Int", cx->any_type, NULL);
  t->equid = equid_imp;
  t->fprint = fprint_imp;

  cx_add_func(cx, "?", cx_arg(t))->ptr = ok_imp;
  
  cx_add_func(cx, "<", cx_arg(t), cx_arg(t))->ptr = lt_imp;
  cx_add_func(cx, ">", cx_arg(t), cx_arg(t))->ptr = gt_imp;

  cx_add_func(cx, "++", cx_arg(t))->ptr = inc_imp;
  cx_add_func(cx, "--", cx_arg(t))->ptr = dec_imp;

  cx_add_func(cx, "+", cx_arg(t), cx_arg(t))->ptr = add_imp;
  cx_add_func(cx, "-", cx_arg(t), cx_arg(t))->ptr = sub_imp;
  cx_add_func(cx, "*", cx_arg(t), cx_arg(t))->ptr = mul_imp;
  cx_add_func(cx, "/", cx_arg(t), cx_arg(t))->ptr = div_imp;

  cx_add_func(cx, "times", cx_arg(t), cx_arg(cx->any_type))->ptr = times_imp;

  return t;
}
