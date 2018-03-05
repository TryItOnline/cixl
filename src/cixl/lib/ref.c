#include <string.h>
#include <inttypes.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/ref.h"
#include "cixl/scope.h"
#include "cixl/lib.h"
#include "cixl/lib/ref.h"

static bool ref_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box v = *cx_test(cx_pop(scope, false));
  struct cx_ref *r = cx_ref_new(cx, NULL);
  r->value = v;
  cx_box_init(cx_push(scope), cx->ref_type)->as_ref = r;
  return true;
}

static bool get_imp(struct cx_scope *scope) {
  struct cx_box r = *cx_test(cx_pop(scope, false));
  cx_copy(cx_push(scope), &r.as_ref->value);
  cx_box_deinit(&r);
  return true;
}

static bool put_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    r = *cx_test(cx_pop(scope, false));

  cx_box_deinit(&r.as_ref->value);
  r.as_ref->value = v;
  cx_box_deinit(&r);
  return true;
}

cx_lib(cx_init_ref, "cx/ref", { 
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc", "A", "Opt");

    cx->ref_type = cx_init_ref_type(lib);

    cx_add_cfunc(lib, "ref",
		 cx_args(cx_arg("val", cx->opt_type)),
		 cx_args(cx_arg(NULL, cx->ref_type)),
		 ref_imp);
  
    cx_add_cfunc(lib, "get-ref",
		 cx_args(cx_arg("ref", cx->ref_type)),
		 cx_args(cx_arg(NULL, cx->opt_type)),
		 get_imp);

    cx_add_cfunc(lib, "put-ref",
		 cx_args(cx_arg("ref", cx->ref_type), cx_arg("val", cx->opt_type)),
		 cx_args(),
		 put_imp);
  })
