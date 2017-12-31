#include <inttypes.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/scope.h"
#include "cixl/types/int.h"

static bool lt_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = x.as_int < y.as_int;
  return true;
}

static bool gt_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));
  
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = x.as_int > y.as_int;
  return true;
}

static bool inc_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = v.as_int+1;
  return true;
}

static bool dec_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = v.as_int-1;
  return true;
}

static bool add_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = x.as_int + y.as_int;
  return true;
}

static bool sub_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = x.as_int - y.as_int;
  return true;
}

static bool mul_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));
  
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = x.as_int * y.as_int;
  return true;
}

static bool div_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  if (!y.as_int) {
    cx_error(cx, cx->row, cx->col, "Division by zero");
    return false;
  }
  
  cx_rat_init(&cx_box_init(cx_push(scope), scope->cx->rat_type)->as_rat,
	      abs(x.as_int), abs(y.as_int),
	      (x.as_int >= 0 || y.as_int > 0) && (x.as_int < 0 || y.as_int < 0));
  
  return true;
}

static bool times_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    reps = *cx_test(cx_pop(scope, false));

  bool ok = false;
  
  for (cx_int_t i = 0; i < reps.as_int; i++) {
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
  
  for (cx_int_t i = 0; i < reps.as_int; i++) {
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

static bool ok_imp(struct cx_box *v) {
  return v->as_int != 0;
}

static void fprint_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "%" PRId64, v->as_int);
}

struct cx_type *cx_init_int_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Int", cx->num_type);
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->fprint = fprint_imp;

  cx_add_func(cx, "<", cx_arg(t), cx_arg(t))->ptr = lt_imp;
  cx_add_func(cx, ">", cx_arg(t), cx_arg(t))->ptr = gt_imp;

  cx_add_func(cx, "++", cx_arg(t))->ptr = inc_imp;
  cx_add_func(cx, "--", cx_arg(t))->ptr = dec_imp;

  cx_add_func(cx, "+", cx_arg(t), cx_arg(t))->ptr = add_imp;
  cx_add_func(cx, "-", cx_arg(t), cx_arg(t))->ptr = sub_imp;
  cx_add_func(cx, "*", cx_arg(t), cx_arg(t))->ptr = mul_imp;
  cx_add_func(cx, "/", cx_arg(t), cx_arg(t))->ptr = div_imp;
  
  cx_add_func(cx, "times", cx_arg(t), cx_arg(cx->any_type))->ptr = times_imp;
  cx_add_func(cx, "for", cx_arg(t), cx_arg(cx->any_type))->ptr = for_imp;
  
  return t;
}
