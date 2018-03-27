#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/iter.h"
#include "cixl/lib.h"
#include "cixl/lib/iter.h"
#include "cixl/scope.h"

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
  if (!cx_call(&it->act, scope)) { return false; }
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
  cx_iter_deref(it->in);
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

struct cx_filter_iter {
  struct cx_iter iter;
  struct cx_iter *in;
  struct cx_box act;
};

bool filter_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_filter_iter *it = cx_baseof(iter, struct cx_filter_iter, iter);

  struct cx_box iv;

  while (true) {
    if (!cx_iter_next(it->in, &iv, scope)) {
      iter->done = it->in->done;
      return false;
    }
    
    cx_copy(cx_push(scope), &iv);
    cx_call(&it->act, scope);
    struct cx_box *ov = cx_pop(scope, true);
    
    if (!ov) {
      cx_error(cx, cx->row, cx->col, "Missing filter value");
      cx_box_deinit(&iv);
      return false;
    }

    if (ov->type != cx->bool_type) {
      cx_error(cx, cx->row, cx->col, "Expected type Bool, actual: %s", ov->type->id);
      cx_box_deinit(&iv);
      return false;
    }

    if (ov->as_bool) { break; }
    cx_box_deinit(&iv);
  }
    
  *out = iv;
  return true;
}

void *filter_deinit(struct cx_iter *iter) {
  struct cx_filter_iter *it = cx_baseof(iter, struct cx_filter_iter, iter);
  cx_iter_deref(it->in);
  cx_box_deinit(&it->act);
  return it;
}

cx_iter_type(filter_iter, {
    type.next = filter_next;
    type.deinit = filter_deinit;
  });

struct cx_filter_iter *cx_filter_iter_new(struct cx_iter *in, struct cx_box *act) {
  struct cx_filter_iter *it = malloc(sizeof(struct cx_filter_iter));
  cx_iter_init(&it->iter, filter_iter());
  it->in = in;
  cx_copy(&it->act, act);
  return it;
}

static bool new_iter_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->iter_type)->as_iter = cx_iter(&v);
  cx_box_deinit(&v);
  return true;
}

static bool for_imp(struct cx_scope *scope) {
  struct cx_box
    act = *cx_test(cx_pop(scope, false)),
    in = *cx_test(cx_pop(scope, false));

  bool ok = false;
  struct cx_iter *it = cx_iter(&in);
  struct cx_box v;
  
  while (cx_iter_next(it, &v, scope)) {
    *cx_push(scope) = v; 
    if (!cx_call(&act, scope)) { goto exit; }
  }

  ok = true;
 exit:
  cx_iter_deref(it);
  cx_box_deinit(&act);
  cx_box_deinit(&in);
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

static bool filter_imp(struct cx_scope *scope) {
  struct cx_box
    act = *cx_test(cx_pop(scope, false)),
    in = *cx_test(cx_pop(scope, false));

  struct cx_iter *it = &cx_filter_iter_new(cx_iter(&in), &act)->iter;
  cx_box_init(cx_push(scope), scope->cx->iter_type)->as_iter = it;

  cx_box_deinit(&act);
  cx_box_deinit(&in);
  return true;
}

static bool next_imp(struct cx_scope *scope) {
  struct cx_box it = *cx_test(cx_pop(scope, false)), v;

  if (cx_iter_next(it.as_iter, &v, scope)) {
    *cx_push(scope) = v;
  } else {
    cx_box_init(cx_push(scope), scope->cx->nil_type);
  }

  cx_box_deinit(&it);
  return true;
}

static bool drop_imp(struct cx_scope *scope) {
  struct cx_box
    n = *cx_test(cx_pop(scope, false)),
    it = *cx_test(cx_pop(scope, false)),
    v;

  bool done = false;
  
  while (!done && n.as_int-- > 0) {
    done = !cx_iter_next(it.as_iter, &v, scope);
    cx_box_deinit(&v);
  }

  cx_box_deinit(&it);
  return true;
}

static bool is_done_imp(struct cx_scope *scope) {
  struct cx_box it = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = it.as_iter->done;
  cx_box_deinit(&it);
  return true;
}

static bool times_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    reps = *cx_test(cx_pop(scope, false));

  bool ok = false;
  
  for (int64_t i = 0; i < reps.as_int; i++) {
    if (!cx_call(&v, scope)) { goto exit; }
  }

  ok = true;
 exit:
  cx_box_deinit(&v);
  return ok;
}

static bool while_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box a = *cx_test(cx_pop(scope, false));
  bool ok = false;
  
  while (true) {
    if (!cx_call(&a, scope)) { goto exit; }
    struct cx_box *vp = cx_pop(scope, false);

    if (!vp) {
      cx_error(cx, cx->row, cx->col, "Missing while condition");
      goto exit;
    }

    struct cx_box v = *vp;
    if (!cx_ok(&v)) { break; }
    cx_box_deinit(&v);
  }

  ok = true;
 exit:
  cx_box_deinit(&a);
  return ok;
}

cx_lib(cx_init_iter, "cx/iter") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Int", "Iter", "Opt", "Seq")) {
    return false;
  }
    
  cx_add_cfunc(lib, "iter",
	       cx_args(cx_arg("seq", cx->seq_type)),
	       cx_args(cx_arg(NULL, cx->iter_type)),
	       new_iter_imp);

  cx_add_cfunc(lib, "for",
	       cx_args(cx_arg("seq", cx->seq_type), cx_arg("act", cx->any_type)),
	       cx_args(),
	       for_imp);
  
  cx_add_cfunc(lib, "map",
	       cx_args(cx_arg("seq", cx->seq_type), cx_arg("act", cx->any_type)),
	       cx_args(cx_arg(NULL, cx->iter_type)),
	       map_imp);
  
  cx_add_cfunc(lib, "filter",
	       cx_args(cx_arg("seq", cx->seq_type), cx_arg("act", cx->any_type)),
	       cx_args(cx_arg(NULL, cx->iter_type)),
	       filter_imp);
  
  cx_add_cfunc(lib, "next",
	       cx_args(cx_arg("it", cx->iter_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       next_imp);

  cx_add_cfunc(lib, "drop", 
	       cx_args(cx_arg("it", cx->iter_type), cx_arg("n", cx->int_type)),
	       cx_args(),
	       drop_imp);

  cx_add_cfunc(lib, "is-done", 
	       cx_args(cx_arg("it", cx->iter_type)),
	       cx_args(cx_arg(NULL, cx->bool_type)),
	       is_done_imp);

  cx_add_cfunc(lib, "times",
	       cx_args(cx_arg("n", cx->int_type),
		       cx_arg("act", cx->any_type)), cx_args(),
	       times_imp);

  cx_add_cfunc(lib, "while",
	       cx_args(cx_arg("act", cx->any_type)), cx_args(),
	       while_imp);

  return true;
}
