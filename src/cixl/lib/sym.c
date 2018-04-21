#include <string.h>

#include "cixl/arg.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/sym.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/sym.h"

static bool sym_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->sym_type)->as_sym = cx_sym(s->cx, v->as_str->data);
  return true;
}

static bool str_imp(struct cx_call *call) {
  struct cx_sym *v = &cx_test(cx_call_arg(call, 0))->as_sym;
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->str_type)->as_str = cx_str_new(v->id, strlen(v->id));
  return true;
}

cx_lib(cx_init_sym, "cx/sym") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "Str", "Sym")) {
    return false;
  }

  cx_add_cfunc(lib, "sym",
	       cx_args(cx_arg("id", cx->str_type)),
	       cx_args(cx_arg(NULL, cx->sym_type)),
	       sym_imp);
    
  cx_add_cfunc(lib, "str",
	       cx_args(cx_arg("s", cx->sym_type)),
	       cx_args(cx_arg(NULL, cx->str_type)),
	       str_imp);

  return true;
}
