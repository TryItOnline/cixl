#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/error.h"
#include "cixl/libs/iter.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/iter.h"

struct cx_map_iter {
  struct cx_iter iter;
  struct cx_iter *in;
  struct cx_box act;
};

bool map_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  struct cx_map_iter *it = cx_baseof(iter, struct cx_map_iter, iter);

  struct cx_box iv;

  if (!cx_iter_next(it->in, &iv, scope)) {
    iter->done = true;
    return false;
  }

  *cx_push(scope) = iv;
  cx_call(&it->act, scope);
  struct cx_box *ov = cx_pop(scope, true);

  if (!ov) {
    struct cx *cx = scope->cx;
    cx_error(cx, cx->row, cx->col, "Missing mapped value");
    return false;
  }

  *out = *ov;
  return true;
}

void *map_deinit(struct cx_iter *iter) {
  struct cx_map_iter *it = cx_baseof(iter, struct cx_map_iter, iter);
  cx_iter_unref(it->in);
  cx_box_deinit(&it->act);
  return it;
}

cx_iter_type(map_iter, {
    type.next = map_next;
    type.deinit = map_deinit;
  });


struct cx_map_iter *cx_map_iter_new(struct cx_iter *in, struct cx_box *act) {
  struct cx_map_iter *it = malloc(sizeof(struct cx_map_iter));
  cx_iter_init(&it->iter, map_iter());
  it->in = in;
  cx_copy(&it->act, act);
  return it;
}

static bool iter_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->iter_type)->as_iter = cx_iter(&v);
  cx_box_deinit(&v);
  return true;
}

static bool for_imp(struct cx_scope *scope) {
  struct cx_box
    act = *cx_test(cx_pop(scope, false)),
    in = *cx_test(cx_pop(scope, false));

  struct cx_iter *it = cx_iter(&in);
  struct cx_box v;
  bool ok = false;
  
  while (cx_iter_next(it, &v, scope)) {
    *cx_push(scope) = v; 
    if (!cx_call(&act, scope)) { goto exit; }
  }
 exit:
  cx_box_deinit(&act);
  cx_box_deinit(&in);
  cx_iter_unref(it);
  return ok;
}

static bool map_imp(struct cx_scope *scope) {
  struct cx_box
    act = *cx_test(cx_pop(scope, false)),
    in = *cx_test(cx_pop(scope, false));

  struct cx_iter *it = &cx_map_iter_new(cx_iter(&in), &act)->iter;
  cx_box_init(cx_push(scope), scope->cx->iter_type)->as_iter = it;

  cx_box_deinit(&act);
  cx_box_deinit(&in);
  return true;
}

void cx_init_iter(struct cx *cx) {
  cx_add_func(cx, "iter", cx_arg(cx->seq_type))->ptr = iter_imp;

  cx_add_func(cx, "for",
	      cx_arg(cx->seq_type),
	      cx_arg(cx->any_type))->ptr = for_imp;

  cx_add_func(cx, "map",
	      cx_arg(cx->seq_type),
	      cx_arg(cx->any_type))->ptr = map_imp;
  
}
