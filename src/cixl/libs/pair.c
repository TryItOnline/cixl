#include <string.h>
#include <inttypes.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/libs/pair.h"
#include "cixl/types/func.h"
#include "cixl/types/fimp.h"
#include "cixl/types/pair.h"

static bool zip_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));

  struct cx_pair *p = cx_pair_new(cx, NULL, NULL);
  p->x = x;
  p->y = y;
  
  cx_box_init(cx_push(scope), cx->pair_type)->as_pair = p;
  return true;
}

static bool unzip_imp(struct cx_scope *scope) {
  struct cx_box *p = cx_test(cx_peek(scope, false)), x, y;
  cx_copy(&x, &p->as_pair->x);
  cx_copy(&y, &p->as_pair->y);
  *cx_box_deinit(p) = x;
  *cx_push(scope) = y;
  return true;
}

static bool x_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  cx_copy(cx_push(scope), &p.as_pair->x);
  cx_box_deinit(&p);
  return true;
}

static bool y_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  cx_copy(cx_push(scope), &p.as_pair->y);
  cx_box_deinit(&p);
  return true;
}

static bool rezip_imp(struct cx_scope *scope) {
  struct cx_box p = *cx_test(cx_pop(scope, false));
  struct cx_box tmp = p.as_pair->x;
  p.as_pair->x = p.as_pair->y;
  p.as_pair->y = tmp;
  cx_box_deinit(&p);
  return true;
}

void cx_init_pair(struct cx *cx) {
  cx_add_cfunc(cx, ".", 
	       cx_args(cx_arg("x", cx->opt_type), cx_arg("y", cx->opt_type)),
	       cx_rets(cx_ret(cx->pair_type)),
	       zip_imp);

  cx_add_cfunc(cx, "unzip", 
	       cx_args(cx_arg("p", cx->pair_type)),
	       cx_rets(cx_ret(cx->opt_type), cx_ret(cx->opt_type)),
	       unzip_imp);

  cx_add_cfunc(cx, "x",
	       cx_args(cx_arg("p", cx->pair_type)),
	       cx_rets(cx_ret(cx->any_type)),
	       x_imp);

  cx_add_cfunc(cx, "y",
	       cx_args(cx_arg("p", cx->pair_type)),
	       cx_rets(cx_ret(cx->any_type)),
	       y_imp);

  cx_add_cfunc(cx, "rezip", 
	       cx_args(cx_arg("p", cx->pair_type)),
	       cx_rets(),
	       rezip_imp);  

  cx_add_cxfunc(cx, "rezip", 
		cx_args(cx_arg("in", cx->seq_type)),
		cx_rets(),
		"$in for &rezip");
}
