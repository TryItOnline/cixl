#include <string.h>
#include <inttypes.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/libs/vect.h"
#include "cixl/types/func.h"
#include "cixl/types/fimp.h"
#include "cixl/types/iter.h"
#include "cixl/types/vect.h"

static bool len_imp(struct cx_scope *scope) {
  struct cx_box vec = *cx_test(cx_pop(scope, false));
  struct cx_vect *v = vec.as_ptr;
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = v->imp.count;
  cx_box_deinit(&vec);
  return true;
}

static bool push_imp(struct cx_scope *scope) {
  struct cx_box
    val = *cx_test(cx_pop(scope, false)),
    vec = *cx_test(cx_pop(scope, false));
  
  struct cx_vect *v = vec.as_ptr;
  *(struct cx_box *)cx_vec_push(&v->imp) = val;
  cx_box_deinit(&vec);
  return true;
}

static bool pop_imp(struct cx_scope *scope) {
  struct cx_box vec = *cx_test(cx_pop(scope, false));
  struct cx_vect *v = vec.as_ptr;

  if (v->imp.count) {
    *cx_push(scope) = *(struct cx_box *)cx_vec_pop(&v->imp);
  } else {
    cx_box_init(cx_push(scope), scope->cx->nil_type);
  }
  
  cx_box_deinit(&vec);
  return true;
}

static bool seq_imp(struct cx_scope *scope) {
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_iter *it = cx_iter(&in);
  struct cx_vect *out = cx_vect_new();
  struct cx_box v;
  
  while (cx_iter_next(it, &v, scope)) {
    *(struct cx_box *)cx_vec_push(&out->imp) = v;
  }

  cx_box_init(cx_push(scope), scope->cx->vect_type)->as_ptr = out;
  cx_box_deinit(&in);
  cx_iter_deref(it);
  return true;
}

static bool clear_imp(struct cx_scope *scope) {
  struct cx_box vec = *cx_test(cx_pop(scope, false));
  struct cx_vect *v = vec.as_ptr;
  cx_vec_clear(&v->imp);
  cx_box_deinit(&vec);
  return true;
}

static bool sort_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    cmp = *cx_test(cx_pop(scope, false)),
    vec = *cx_test(cx_pop(scope, false));

  struct cx_vect *v = vec.as_ptr;
  struct cx_sym lt = cx_sym(cx, "lt"), gt = cx_sym(cx, "gt");
  bool ok = false;
  
  int do_cmp(const void *x, const void *y) {
    const struct cx_box *xv = x, *yv = y;
    int res = 0;
    
    if (cmp.type == cx->nil_type) {
      if (scope->safe && !(cx_is(xv->type, yv->type) || cx_is(yv->type, xv->type))) {
	cx_error(cx, cx->row, cx->col,
		 "Failed comparing %s to %s", xv->type->id, yv->type->id);
	return 0;
      }

      switch(cx_cmp(xv, yv)) {
      case CX_CMP_LT:
	res = -1;
	break;
      case CX_CMP_GT:
	res = 1;
	break;
      default:
	break;
      }
    } else {
      cx_copy(cx_push(scope), xv);
      cx_copy(cx_push(scope), yv);
      if (!cx_call(&cmp, scope)) { return 0; }
      struct cx_box *out = cx_pop(scope, false);
      if (!out) { return 0; }

      if (out->type != cx->sym_type) {
	cx_error(cx, cx->row, cx->col, "Expected Sym, actual: %s", out->type->id);
	cx_box_deinit(out);
	return 0;
      }
      
      if (out->as_sym.tag == lt.tag) {
	res = -1;
      } else if (out->as_sym.tag == gt.tag) {
	res = 1;
      }
    }
    
    return res;
  }

  qsort(v->imp.items, v->imp.count, v->imp.item_size, do_cmp);
  ok = !cx->errors.count;
  cx_box_deinit(&cmp);
  cx_box_deinit(&vec);
  return ok;
}

void cx_init_vect(struct cx *cx) {
  cx_add_cfunc(cx, "len",
	       cx_args(cx_arg("vec", cx->vect_type)),
	       cx_rets(cx_ret(cx->int_type)),
	       len_imp);
  
  cx_add_cfunc(cx, "push",
	       cx_args(cx_arg("vec", cx->vect_type), cx_arg("val", cx->any_type)),
	       cx_rets(),
	       push_imp);

  cx_add_cfunc(cx, "pop",
	       cx_args(cx_arg("vec", cx->vect_type)), cx_rets(cx_ret(cx->opt_type)),
	       pop_imp);

  cx_add_cfunc(cx, "vect",
	       cx_args(cx_arg("in", cx->seq_type)),
	       cx_rets(cx_ret(cx->vect_type)),
	       seq_imp);

  cx_add_cfunc(cx, "clear",
	       cx_args(cx_arg("vec", cx->vect_type)), cx_rets(),
	       clear_imp);

  cx_add_cfunc(cx, "sort",
	       cx_args(cx_arg("vec", cx->vect_type), cx_arg("cmp", cx->opt_type)),
	       cx_rets(),
	       sort_imp);
}
