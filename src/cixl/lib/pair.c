#include <string.h>
#include <inttypes.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/pair.h"
#include "cixl/scope.h"
#include "cixl/lib.h"
#include "cixl/lib/pair.h"

static bool zip_imp(struct cx_call *call) {
  struct cx_box
    *a = cx_test(cx_call_arg(call, 0)),
    *b = cx_test(cx_call_arg(call, 1));

  struct cx_scope *s = call->scope;
  struct cx_pair *p = cx_pair_new(s->cx, NULL, NULL);
  cx_copy(&p->a, a);
  cx_copy(&p->b, b);

  struct cx_type
    *at = (a->type == s->cx->nil_type) ? s->cx->opt_type : a->type,
    *bt = (b->type == s->cx->nil_type) ? s->cx->opt_type : b->type;
    
  cx_box_init(cx_push(s), cx_type_get(s->cx->pair_type, at, bt))->as_pair = p;
  return true;
}

static bool splat_imp(struct cx_call *call) {
  struct cx_box *p = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_copy(cx_push(s), &p->as_pair->a);
  cx_copy(cx_push(s), &p->as_pair->b);
  return true;
}

static bool a_imp(struct cx_call *call) {
  struct cx_box *p = cx_test(cx_call_arg(call, 0));
  cx_copy(cx_push(call->scope), &p->as_pair->a);
  return true;
}

static bool b_imp(struct cx_call *call) {
  struct cx_box *p = cx_test(cx_call_arg(call, 0));
  cx_copy(cx_push(call->scope), &p->as_pair->b);
  return true;
}

static bool rezip_imp(struct cx_call *call) {
  struct cx_box *pv = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  struct cx_pair *p = pv->as_pair;
  struct cx_box tmp = p->a;
  p->a = p->b;
  p->b = tmp;
  struct cx_type *pt = cx_test(cx_subtype(pv->type, s->cx->pair_type));
      
  cx_box_init(cx_push(s), cx_type_get(s->cx->pair_type,
				      cx_type_arg(pt, 1),
				      cx_type_arg(pt, 0)))->as_pair =
    cx_pair_ref(p);

  return true;
}

cx_lib(cx_init_pair, "cx/pair") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Cmp", "Opt", "Seq") ||
      !cx_use(cx, "cx/iter", "for")) {
    return false;
  }

  cx->pair_type = cx_init_pair_type(lib);
    
  cx_add_cfunc(lib, ",", 
	       cx_args(cx_arg("a", cx->opt_type), cx_arg("b", cx->opt_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->pair_type,
						cx_arg_ref(cx, 0),
						cx_arg_ref(cx, 1)))),
	       zip_imp);

  cx_add_cfunc(lib, "..", 
	       cx_args(cx_arg("p", cx->pair_type)),
	       cx_args(cx_narg(cx, NULL, 0, 0), cx_narg(cx, NULL, 0, 1)),
	       splat_imp);

  cx_add_cfunc(lib, "a",
	       cx_args(cx_arg("p", cx->pair_type)),
	       cx_args(cx_narg(cx, NULL, 0, 0)),
	       a_imp);

  cx_add_cfunc(lib, "b",
	       cx_args(cx_arg("p", cx->pair_type)),
	       cx_args(cx_narg(cx, NULL, 0, 1)),
	       b_imp);

  cx_add_cfunc(lib, "rezip", 
	       cx_args(cx_arg("p", cx->pair_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->pair_type,
						cx_arg_ref(cx, 0, 1),
						cx_arg_ref(cx, 0, 0)))),
	       rezip_imp);

  cx_add_cxfunc(lib, "rezip", 
		cx_args(cx_arg("in", cx->seq_type)),
		cx_args(cx_arg(NULL, cx->iter_type)),
		"$in &rezip map");

  return true;
}
