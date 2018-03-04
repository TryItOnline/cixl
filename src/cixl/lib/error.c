#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/lib.h"
#include "cixl/lib/error.h"

static bool check_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  struct cx *cx = scope->cx;
  bool ok = true;
  
  if (!cx_ok(&v)) {
    cx_error(cx, cx->row, cx->col, "Check failed");
    ok = false;
  }

  cx_box_deinit(&v);
  return ok;
}

static bool fail_imp(struct cx_scope *scope) {
  struct cx_box m = *cx_test(cx_pop(scope, false));
  struct cx *cx = scope->cx;
  cx_error(cx, cx->row, cx->col, m.as_str->data);
  cx_box_deinit(&m);
  return false;
}

cx_lib(cx_init_error, "cx/error", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");
    cx_use(cx, "cx/str/types");

    cx_add_cfunc(lib, "check",
		 cx_args(cx_arg("v", cx->opt_type)), cx_args(),
		 check_imp);
    
    cx_add_cfunc(lib, "fail",
		 cx_args(cx_arg("msg", cx->str_type)), cx_args(),
		 fail_imp);
  })
