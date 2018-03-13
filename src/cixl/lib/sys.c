#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/sys.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static bool home_dir_imp(struct cx_scope *scope) {
  cx_box_init(cx_push(scope), scope->cx->str_type)->as_str =
    cx_str_new(cx_home_dir());
  
  return true;
}

static bool make_dir_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  bool ok = cx_make_dir(p.as_str->data);
  cx_box_deinit(&p);
  return ok;
}

cx_lib(cx_init_sys, "cx/sys") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "Str")) {
    return false;
  }

  cx_add_cfunc(lib, "home-dir",
	       cx_args(),
	       cx_args(cx_arg(NULL, cx->str_type)),
	       home_dir_imp);

  cx_add_cfunc(lib, "make-dir",
	       cx_args(cx_arg("p", cx->str_type)),
	       cx_args(),
	       make_dir_imp);

  return true;
}
