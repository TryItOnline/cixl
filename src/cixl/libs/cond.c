#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/libs/cond.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

static bool eqval_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));
  
  cx_box_init(cx_push(scope),
	      scope->cx->bool_type)->as_bool = cx_eqval(&x, &y);
  
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool equid_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));
  
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = cx_equid(&x, &y);
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  return true;
}

static bool ok_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box v = *cx_test(cx_peek(scope, false));
  if (v.type == cx->bool_type) { return true; }
  cx_pop(scope, false);
  cx_box_init(cx_push(scope), cx->bool_type)->as_bool = cx_ok(&v);
  cx_box_deinit(&v);
  return true;
}

static bool not_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box *v = cx_test(cx_peek(scope, false));

  if (v->type == cx->bool_type) {
    v->as_bool = !v->as_bool;
    return true;
  }
  
  bool ok = cx_ok(v);
  cx_box_deinit(v);  
  cx_pop(scope, false);
  cx_box_init(cx_push(scope), cx->bool_type)->as_bool = !ok;
  return true;
}

static bool and_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));
  
  if (!cx_call(&x, scope)) {
    cx_box_deinit(&y);
    return false;
  }
  
  cx_box_deinit(&x);
  struct cx_box *xc = cx_pop(scope, false);
  
  if (!xc) {
    cx_error(cx, cx->row, cx->col, "Missing argument");
    cx_box_deinit(&y);
    return false;    
  }

  struct cx_box xcv = *xc;
  
  if (!cx_ok(&xcv)) {
    cx_box_deinit(&y);
    *cx_push(scope) = xcv;
    return true;
  }
  
  cx_box_deinit(&xcv);
  
  if (!cx_call(&y, scope)) {
    cx_box_deinit(&y);
    return false;
  }
  
  cx_box_deinit(&y);
  struct cx_box *yc = cx_pop(scope, false);

  if (!yc) {
    cx_error(cx, cx->row, cx->col, "Missing argument");
    return false;    
  }

  struct cx_box ycv = *yc;
  *cx_push(scope) = ycv;
  return true;
}

static bool or_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;

  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false));
  
  if (!cx_call(&x, scope)) {
    cx_box_deinit(&x);
    cx_box_deinit(&y);
    return false;
  }
  
  cx_box_deinit(&x);
  struct cx_box *xc = cx_pop(scope, false);

  if (!xc) {
    cx_error(cx, cx->row, cx->col, "Missing argument");
    cx_box_deinit(&y);
    return false;    
  }

  struct cx_box xcv = *xc;
  
  if (cx_ok(&xcv)) {
    cx_box_deinit(&y);
    *cx_push(scope) = xcv;
    return true;
  }
  
  cx_box_deinit(&xcv);
  
  if (!cx_call(&y, scope)) {
    cx_box_deinit(&y);
    return false;
  }
  
  cx_box_deinit(&y);
  struct cx_box *yc = cx_pop(scope, false);

  if (!yc) {
    cx_error(cx, cx->row, cx->col, "Missing argument");
    return false;    
  }
  
  struct cx_box ycv = *yc;
  *cx_push(scope) = ycv;
  return true;
}

static bool if_imp(struct cx_scope *scope) {
  struct cx_box
    x = *cx_test(cx_pop(scope, false)),
    c = *cx_test(cx_pop(scope, false));
  
  if (cx_ok(&c)) { cx_call(&x, scope); }
  cx_box_deinit(&x);
  cx_box_deinit(&c);
  return true;
}

static bool else_imp(struct cx_scope *scope) {
  struct cx_box
    x = *cx_test(cx_pop(scope, false)),
    c = *cx_test(cx_pop(scope, false));
  
  if (!cx_ok(&c)) { cx_call(&x, scope); }
  cx_box_deinit(&x);
  cx_box_deinit(&c);
  return true;
}

static bool if_else_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    x = *cx_test(cx_pop(scope, false)),
    c = *cx_test(cx_pop(scope, false));
  
  cx_call(cx_ok(&c) ? &x : &y, scope);
  cx_box_deinit(&x);
  cx_box_deinit(&y);
  cx_box_deinit(&c);
  return true;
}

void cx_init_cond(struct cx *cx) {
  cx_add_cfunc(cx, "=", eqval_imp, cx_arg("x", cx->opt_type), cx_narg("y", 0));
  cx_add_cfunc(cx, "==", equid_imp, cx_arg("x", cx->opt_type), cx_narg("y", 0));

  cx_add_cfunc(cx, "?", ok_imp, cx_arg("v", cx->opt_type));
  cx_add_cfunc(cx, "!", not_imp, cx_arg("v", cx->opt_type));
  
  cx_add_cfunc(cx, "and", and_imp,
	       cx_arg("x", cx->opt_type), cx_arg("y", cx->opt_type));
  
  cx_add_cfunc(cx, "or", or_imp,
	       cx_arg("x", cx->opt_type), cx_arg("y", cx->opt_type));
  
  cx_add_cfunc(cx, "if", if_imp,
	       cx_arg("cnd", cx->opt_type), cx_arg("act", cx->any_type));

  cx_add_cfunc(cx, "else", else_imp,
	       cx_arg("cnd", cx->opt_type), cx_arg("act", cx->any_type));

  cx_add_cfunc(cx, "if-else", if_else_imp,
	       cx_arg("cnd", cx->opt_type),
	       cx_arg("tact", cx->any_type), cx_arg("fact", cx->any_type));
}
