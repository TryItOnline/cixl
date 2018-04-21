#include "cixl/arg.h"
#include "cixl/call.h"
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

static bool new_iter_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_iter(v, cx_push(s));
  return true;
}

static bool for_imp(struct cx_call *call) {
  struct cx_box
    *act = cx_test(cx_call_arg(call, 1)),
    *in = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  struct cx_box it;
  cx_iter(in, &it);
  struct cx_box v;
  bool ok = false;
  
  while (cx_iter_next(it.as_iter, &v, s)) {
    *cx_push(s) = v; 
    if (!cx_call(act, s)) { goto exit; }
  }

  ok = true;
 exit:
  cx_box_deinit(&it);
  return ok;
}

static bool map_imp(struct cx_call *call) {
  struct cx_box
    *act = cx_test(cx_call_arg(call, 1)),
    *in = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  struct cx_box in_it;
  cx_iter(in, &in_it);
  struct cx_iter *it = &cx_map_iter_new(in_it.as_iter, act)->iter;
  cx_box_init(cx_push(s), s->cx->iter_type)->as_iter = it;
  return true;
}

static bool filter_imp(struct cx_call *call) {
  struct cx_box
    *act = cx_test(cx_call_arg(call, 1)),
    *in = cx_test(cx_call_arg(call, 0));

  struct cx_box in_it;
  cx_iter(in, &in_it);
  struct cx_iter *it = &cx_filter_iter_new(in_it.as_iter, act)->iter;
  cx_box_init(cx_push(call->scope), in_it.type)->as_iter = it;
  return true;
}

static bool next_imp(struct cx_call *call) {
  struct cx_box *it = cx_test(cx_call_arg(call, 0)), v;
  struct cx_scope *s = call->scope;
  
  if (cx_iter_next(it->as_iter, &v, s)) {
    *cx_push(s) = v;
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }

  return true;
}

static bool drop_imp(struct cx_call *call) {
  struct cx_box
    *n = cx_test(cx_call_arg(call, 1)),
    *it = cx_test(cx_call_arg(call, 0)),
    v;

  struct cx_scope *s = call->scope;
  
  while (n->as_int-- > 0) {
    if (!cx_iter_next(it->as_iter, &v, s)) {
      cx_error(s->cx, s->cx->row, s->cx->col, "Failed dropping");
      return false;
    }
      
    cx_box_deinit(&v);
  }

  return true;
}

static bool is_done_imp(struct cx_call *call) {
  struct cx_box *it = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->bool_type)->as_bool = it->as_iter->done;
  return true;
}

static bool times_imp(struct cx_call *call) {
  struct cx_box
    *v = cx_test(cx_call_arg(call, 1)),
    *reps = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  
  for (int64_t i = 0; i < reps->as_int; i++) {
    if (!cx_call(v, s)) { return false; }
  }

  return true;
}

static bool while_imp(struct cx_call *call) {
  struct cx_box *a = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  
  while (true) {
    if (!cx_call(a, s)) { return false; }
    struct cx_box *vp = cx_pop(s, false);

    if (!vp) {
      cx_error(s->cx, s->cx->row, s->cx->col, "Missing while condition");
      return false;
    }

    struct cx_box v = *vp;
    bool ok = cx_ok(&v);
    cx_box_deinit(&v);
    if (!ok) { break; }
  }

  return true;
}

static bool find_if_imp(struct cx_call *call) {
  struct cx_box
    *pred = cx_test(cx_call_arg(call, 1)),
    *in = cx_test(cx_call_arg(call, 0)),
    it;

  struct cx_scope *s = call->scope;
  cx_iter(in, &it);
  struct cx_box iv;
  bool ok = false;
  
  while (cx_iter_next(it.as_iter, &iv, s)) {
    cx_copy(cx_push(s), &iv);
    if (!cx_call(pred, s)) { goto exit; }
    struct cx_box *ovp = cx_pop(s, false);
    if (!ovp) { goto exit; }
    struct cx_box ov = *ovp;
    bool found = cx_ok(&ov);
    cx_box_deinit(&ov);

    if (found) {
      *cx_push(s) = iv;
      ok = true;
      goto exit;
    }

    cx_box_deinit(&iv);
  }
  
  cx_box_init(cx_push(s), s->cx->nil_type);
 exit:
  cx_box_deinit(&it);
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
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx_arg_ref(cx, 0, 0)))),
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

  cx_add_cfunc(lib, "find-if",
	       cx_args(cx_arg("in", cx->seq_type),
		       cx_arg("pred", cx->any_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx_arg_ref(cx, 0, 0)))),
	       find_if_imp);
  
  return true;
}
