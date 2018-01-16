#include <stdlib.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/call_iter.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/iter.h"

struct cx_call_iter {
  struct cx_iter iter;
  struct cx_box target;
};

bool call_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_call_iter *it = cx_baseof(iter, struct cx_call_iter, iter);

  cx_call(&it->target, scope);
  struct cx_box *res = cx_pop(scope, true);
  
  if (!res) {
    cx_error(cx, cx->row, cx->col, "Missing iter result");
    return false;
  }

  if (res->type == cx->nil_type) {
    iter->done = true;
    return false;
  }
  
  *out = *res;
  return true;
}

void *call_deinit(struct cx_iter *iter) {
  struct cx_call_iter *it = cx_baseof(iter, struct cx_call_iter, iter);
  cx_box_deinit(&it->target);
  return it;
}

cx_iter_type(call_iter, {
    type.next = call_next;
    type.deinit = call_deinit;
  });

struct cx_iter *cx_call_iter_new(struct cx_box *target) {
  struct cx_call_iter *it = malloc(sizeof(struct cx_call_iter));
  cx_iter_init(&it->iter, call_iter());
  cx_copy(&it->target, target);
  return &it->iter;
}
