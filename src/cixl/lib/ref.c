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
  
  cx_box_init(cx_push(scope),
	      (v.type == cx->nil_type)
	      ? cx->ref_type
	      : cx_type_get(cx->ref_type, v.type))->as_ref = r;
  
  return true;
}

static bool deref_imp(struct cx_scope *scope) {
  struct cx_box r = *cx_test(cx_pop(scope, false));
  cx_copy(cx_push(scope), &r.as_ref->value);
  cx_box_deinit(&r);
  return true;
}

static bool set_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    r = *cx_test(cx_pop(scope, false));

  cx_box_deinit(&r.as_ref->value);
  r.as_ref->value = v;
  cx_box_deinit(&r);
  return true;
}

static bool set_call_imp(struct cx_scope *scope) {
  struct cx_box
    a = *cx_test(cx_pop(scope, false)),
    r = *cx_test(cx_pop(scope, false));

  bool ok = false;
  cx_copy(cx_push(scope), &r.as_ref->value);
  if (!cx_call(&a, scope)) { goto exit; }
  struct cx_box *v = cx_pop(scope, false);

  if (v) {
    cx_box_deinit(&r.as_ref->value);
    r.as_ref->value = *v;
  }

  ok = true;
 exit:
  cx_box_deinit(&a);
  cx_box_deinit(&r);
  return ok;
}

cx_lib(cx_init_ref, "cx/ref") { 
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Opt") ||
      !cx_use(cx, "cx/type", "new")) {
    return false;
  }

  cx->ref_type = cx_init_ref_type(lib);

  cx_add_cfunc(lib, "ref",
	       cx_args(cx_arg("val", cx->opt_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->ref_type, cx_arg_ref(cx, 0, 0)))),
	       ref_imp);
  
  cx_add_cfunc(lib, "deref",
	       cx_args(cx_arg("ref", cx->ref_type)),
	       cx_args(cx_narg(cx, NULL, 0, 0)),
	       deref_imp);

  cx_add_cfunc(lib, "set",
	       cx_args(cx_arg("ref", cx->ref_type),
		       cx_narg(cx, "val", 0, 0)),
	       cx_args(),
	       set_imp);

  cx_add_cfunc(lib, "set-call",
	       cx_args(cx_arg("ref", cx->ref_type), cx_arg("act", cx->any_type)),
	       cx_args(),
	       set_call_imp);
  
  return true;
}
