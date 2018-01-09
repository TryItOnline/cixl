#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/cmp.h"
#include "cixl/scope.h"

struct cx_sym to_sym(struct cx *cx, enum cx_cmp cmp) {
  switch (cmp) {
  case CX_CMP_LT:
    return cx_sym(cx, "lt");
  case CX_CMP_EQ:
    return cx_sym(cx, "eq");
  default:
    break;
  }

  return cx_sym(cx, "gt");
}

static bool cmp_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  cx_box_init(cx_push(scope), cx->sym_type)->as_sym = to_sym(cx, cx_cmp(&x, &y));
  return true;
}

static bool lt_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  cx_box_init(cx_push(scope),
	      scope->cx->bool_type)->as_bool = cx_cmp(&x, &y) == CX_CMP_LT;
  
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool gt_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  cx_box_init(cx_push(scope),
	      scope->cx->bool_type)->as_bool = cx_cmp(&x, &y) == CX_CMP_GT;
  
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool lte_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  enum cx_cmp cmp = cx_cmp(&x, &y);
  
  cx_box_init(cx_push(scope),
	      scope->cx->bool_type)->as_bool = cmp == CX_CMP_LT || cmp == CX_CMP_EQ;
  
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool gte_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  enum cx_cmp cmp = cx_cmp(&x, &y);
  
  cx_box_init(cx_push(scope),
	      scope->cx->bool_type)->as_bool = cmp == CX_CMP_GT || cmp == CX_CMP_EQ;
  
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

struct cx_type *cx_init_cmp_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Cmp", cx->any_type);
  t->trait = true;

  cx_add_func(cx, "cmp", cx_arg(t), cx_narg(0))->ptr = cmp_imp;
  cx_add_func(cx, "<", cx_arg(t), cx_narg(0))->ptr = lt_imp;
  cx_add_func(cx, ">", cx_arg(t), cx_narg(0))->ptr = gt_imp;
  cx_add_func(cx, "<=", cx_arg(t), cx_narg(0))->ptr = lte_imp;
  cx_add_func(cx, ">=", cx_arg(t), cx_narg(0))->ptr = gte_imp;
  
  return t;
}
