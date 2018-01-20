#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/libs/stack.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

static bool reset_imp(struct cx_scope *scope) {
  cx_do_vec(&scope->stack, struct cx_box, v) { cx_box_deinit(v); }
  cx_vec_clear(&scope->stack);
  return true;
}

static bool zap_imp(struct cx_scope *scope) {
  cx_box_deinit(cx_test(cx_pop(scope, false)));
  return true;
}

static bool copy_imp(struct cx_scope *scope) {
  cx_copy(cx_push(scope), cx_test(cx_peek(scope, true)));
  return true;
}

static bool clone_imp(struct cx_scope *scope) {
  cx_clone(cx_push(scope), cx_test(cx_peek(scope, true)));
  return true;
}

static bool flip_imp(struct cx_scope *scope) {
  if (scope->stack.count < 2) {
    struct cx *cx = scope->cx;
    cx_error(cx, cx->row, cx->col, "Nothing to flip");
    return false;
  }

  struct cx_box *ptr = cx_vec_peek(&scope->stack, 0), tmp = *ptr;
  *ptr = *(ptr-1);
  *(ptr-1) = tmp;
  return true;
}

void cx_init_stack(struct cx *cx) {
  cx_add_cfunc(cx, "|", reset_imp);
  cx_add_cfunc(cx, "_", zap_imp);
  cx_add_cfunc(cx, "%", copy_imp, cx_arg("v", cx->opt_type));
  cx_add_cfunc(cx, "%%", clone_imp, cx_arg("v", cx->opt_type));
  cx_add_cfunc(cx, "~", flip_imp, cx_arg("v", cx->opt_type));
}
