#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/scope.h"
#include "cixl/libs/math.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

static bool int_add_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    *x = cx_test(cx_peek(scope, false));

  x->as_int += y.as_int;
  return true;
}

static bool int_sub_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    *x = cx_test(cx_peek(scope, false));

  x->as_int -= y.as_int;
  return true;
}

static bool int_mul_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    *x = cx_test(cx_peek(scope, false));

  x->as_int *= y.as_int;
  return true;
}

static bool int_div_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  if (!y.as_int) {
    cx_error(cx, cx->row, cx->col, "Division by zero");
    return false;
  }
  
  cx_rat_init(&cx_box_init(cx_push(scope), cx->rat_type)->as_rat,
	      cx_abs(x.as_int), cx_abs(y.as_int),
	      (x.as_int >= 0 || y.as_int > 0) && (x.as_int < 0 || y.as_int < 0));
  
  return true;
}

static bool rand_imp(struct cx_scope *scope) {
  struct cx_box max = *cx_test(cx_pop(scope, false));
  int64_t out = 0;
  int32_t *p = (int *)&out;
  *p++ = rand();
  *p = rand();
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = out % max.as_int;
  return true;
}

static bool rat_add_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    *x = cx_test(cx_peek(scope, false));

  cx_rat_add(&x->as_rat, &y.as_rat);
  return true;
}

static bool rat_mul_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    *x = cx_test(cx_peek(scope, false));

  cx_rat_mul(&x->as_rat, &y.as_rat);
  return true;
}

void cx_init_math(struct cx *cx) {
  cx_add_cfunc(cx, "+", int_add_imp,
	       cx_arg("x", cx->int_type), cx_arg("y", cx->int_type));
  cx_add_cfunc(cx, "-", int_sub_imp,
	       cx_arg("x", cx->int_type), cx_arg("y", cx->int_type));
  cx_add_cfunc(cx, "*", int_mul_imp,
	       cx_arg("x", cx->int_type), cx_arg("y", cx->int_type));
  cx_add_cfunc(cx, "/", int_div_imp,
	       cx_arg("x", cx->int_type), cx_arg("y", cx->int_type));

  cx_add_cfunc(cx, "rand", rand_imp, cx_arg("n", cx->int_type));

  cx_add_cfunc(cx, "+", rat_add_imp,
	       cx_arg("x", cx->rat_type), cx_arg("y", cx->rat_type));
  cx_add_cfunc(cx, "*", rat_mul_imp,
	       cx_arg("x", cx->rat_type), cx_arg("y", cx->rat_type));

  cx_add_func(cx, "fib-rec",
	      "$n? if-else {$b $a $b + $n -- recall} $a",
	      cx_arg("a", cx->int_type),
	      cx_arg("b", cx->int_type),
	      cx_arg("n", cx->int_type));

  cx_add_func(cx, "fib",
	      "0 1 $n fib-rec",
	      cx_arg("n", cx->int_type));

  cx_add_func(cx, "sum",
	      "0, $in for &+",
	      cx_arg("in", cx->seq_type));
}
