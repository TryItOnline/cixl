#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/sym.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/sym.h"

static bool sym_imp(struct cx_scope *scope) {
  struct cx_box s = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope),
	      scope->cx->sym_type)->as_sym = cx_sym(scope->cx, s.as_str->data);
  cx_box_deinit(&s);
  return true;
}

static bool str_imp(struct cx_scope *scope) {
  struct cx_sym s = cx_test(cx_pop(scope, false))->as_sym;
  cx_box_init(cx_push(scope), scope->cx->str_type)->as_str = cx_str_new(s.id);
  return true;
}

cx_lib(cx_init_sym, "cx/sym", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc", "A");
    cx_use(cx, "cx/str", "Str");

    cx->sym_type = cx_init_sym_type(lib);

    cx_add_cfunc(lib, "sym",
		 cx_args(cx_arg("id", cx->str_type)),
		 cx_args(cx_arg(NULL, cx->sym_type)),
		 sym_imp);
    
    cx_add_cfunc(lib, "str",
		 cx_args(cx_arg("s", cx->sym_type)),
		 cx_args(cx_arg(NULL, cx->str_type)),
		 str_imp);
  })
