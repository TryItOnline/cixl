#include <inttypes.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/iter.h"
#include "cixl/lib.h"
#include "cixl/lib/stack.h"
#include "cixl/scope.h"
#include "cixl/stack.h"

static bool len_imp(struct cx_call *call) {
  struct cx_stack *st = cx_test(cx_call_arg(call, 0))->as_ptr;
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = st->imp.count;
  return true;
}

static bool push_imp(struct cx_call *call) {
  struct cx_box *val = cx_test(cx_call_arg(call, 1));
  struct cx_stack *st = cx_test(cx_call_arg(call, 0))->as_ptr;
  cx_copy(cx_vec_push(&st->imp), val);
  return true;
}

static bool put_imp(struct cx_call *call) {
  struct cx_box
    *val = cx_test(cx_call_arg(call, 2)),
    *i = cx_test(cx_call_arg(call, 1));
  
  struct cx_stack *st = cx_test(cx_call_arg(call, 0))->as_ptr;
  struct cx_scope *s = call->scope;
  
  if (i->as_int < 0 || i->as_int >= st->imp.count) {
    cx_error(s->cx, s->cx->row, s->cx->col,
	     "Index out of bounds: %" PRId64,
	     i->as_int);
    
    return false;
  }

  struct cx_box *p = cx_vec_get(&st->imp, i->as_int);
  cx_box_deinit(p);
  cx_copy(p, val);
  return true;
}

static bool pop_imp(struct cx_call *call) {
  struct cx_stack *st = cx_test(cx_call_arg(call, 0))->as_ptr;
  struct cx_scope *s = call->scope;
  
  if (st->imp.count) {
    *cx_push(s) = *(struct cx_box *)cx_vec_pop(&st->imp);
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }
  
  return true;
}

static bool get_imp(struct cx_call *call) {
  struct cx_box *i = cx_test(cx_call_arg(call, 1));
  struct cx_stack *st = cx_test(cx_call_arg(call, 0))->as_ptr;
  struct cx_scope *s = call->scope;
  
  if (i->as_int < st->imp.count) {
    cx_copy(cx_push(s), (struct cx_box *)cx_vec_get(&st->imp, i->as_int));
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }

  return true;
}

static bool last_imp(struct cx_call *call) {
  struct cx_stack *st = cx_test(cx_call_arg(call, 0))->as_ptr;
  struct cx_scope *s = call->scope;
  
  if (st->imp.count) {
    cx_copy(cx_push(s), (struct cx_box *)cx_vec_peek(&st->imp, 0));
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }

  return true;
}

static bool get_rand_imp(struct cx_call *call) {
  struct cx_stack *st = cx_test(cx_call_arg(call, 0))->as_ptr;
  struct cx_scope *s = call->scope;
  
  if (st->imp.count) {
    int64_t i = cx_rand(st->imp.count);
    cx_copy(cx_push(s), (struct cx_box *)cx_vec_get(&st->imp, i));
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }
  
  return true;
}

static bool stack_imp(struct cx_call *call) {
  struct cx_box *in = cx_test(cx_call_arg(call, 0)), it;
  struct cx_scope *s = call->scope;
  cx_iter(in, &it);
  struct cx_stack *out = cx_stack_new(s->cx);
  struct cx_box v;
  struct cx_type *t = NULL;
  
  while (cx_iter_next(it.as_iter, &v, s)) {
    *(struct cx_box *)cx_vec_push(&out->imp) = v;
    t = t ? cx_super(t, v.type) : v.type;
  }

  cx_box_init(cx_push(s),
	      t ? cx_type_get(s->cx->stack_type, t) : s->cx->stack_type)->as_ptr =
    out;
  
  cx_box_deinit(&it);
  return true;
}

static bool stash_imp(struct cx_call *call) {
  cx_stash(call->scope);
  return true;
}

static bool splat_imp(struct cx_call *call) {
  struct cx_box *in = cx_test(cx_call_arg(call, 0)), it;
  struct cx_scope *s = call->scope;
  cx_iter(in, &it);
  struct cx_box v;
  while (cx_iter_next(it.as_iter, &v, s)) { *cx_push(s) = v; }
  cx_box_deinit(&it);
  return true;
}

static bool clear_imp(struct cx_call *call) {
  struct cx_stack *st = cx_test(cx_call_arg(call, 0))->as_ptr;
  cx_vec_clear(&st->imp);
  return true;
}

static bool sort_imp(struct cx_call *call) {
  struct cx_box *cmp = cx_test(cx_call_arg(call, 1));
  struct cx_stack *st = cx_test(cx_call_arg(call, 0))->as_ptr;
  struct cx_scope *s = call->scope;
  struct cx_sym lt = cx_sym(s->cx, "<"), gt = cx_sym(s->cx, ">");
  
  int do_cmp(const void *x, const void *y) {
    const struct cx_box *xv = x, *yv = y;
    int res = 0;
    
    if (cmp->type == s->cx->nil_type) {
      if (s->safe && !(cx_is(xv->type, yv->type) || cx_is(yv->type, xv->type))) {
	cx_error(s->cx, s->cx->row, s->cx->col,
		 "Failed comparing %s to %s",
		 xv->type->id, yv->type->id);
	
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
      cx_copy(cx_push(s), xv);
      cx_copy(cx_push(s), yv);
      if (!cx_call(cmp, s)) { return 0; }
      struct cx_box *out = cx_pop(s, false);
      if (!out) { return 0; }

      if (out->type != s->cx->sym_type) {
	cx_error(s->cx, s->cx->row, s->cx->col,
		 "Expected Sym, actual: %s",
		 out->type->id);
	
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

  qsort(st->imp.items, st->imp.count, st->imp.item_size, do_cmp);
  return true;
}

static bool fill_imp(struct cx_call *call) {
  struct cx_box
    *act = cx_test(cx_call_arg(call, 2)),
    *n = cx_test(cx_call_arg(call, 1));

  struct cx_stack *st = cx_test(cx_call_arg(call, 0))->as_ptr;
  struct cx_scope *s = call->scope;
  if (!n->as_int) { return true; }
  cx_vec_grow(&st->imp, st->imp.count+n->as_int);
  
  for (int64_t i=0; i<n->as_int; i++) {
    if (!cx_call(act, s)) { return false; }
    struct cx_box *v = cx_pop(s, false);
    if (!v) { return false; }
    *(struct cx_box *)cx_vec_push(&st->imp) = *v;
  }

  return true;
}

static bool move_imp(struct cx_call *call) {
  struct cx_box
    *delta = cx_test(cx_call_arg(call, 3)),
    *len = cx_test(cx_call_arg(call, 2)),
    *start = cx_test(cx_call_arg(call, 1));

  struct cx_stack *st = cx_test(cx_call_arg(call, 0))->as_ptr;
  struct cx_scope *s = call->scope;
    
  if (start->as_int+delta->as_int < 0 || start->as_int+len->as_int > st->imp.count) {
    cx_error(s->cx, s->cx->row, s->cx->col, "Move out of bounds");
    return false;
  }

  size_t prev_count = st->imp.count;
  
  if (delta->as_int > 0) {
    size_t n = start->as_int+len->as_int+delta->as_int;
    cx_vec_grow(&st->imp, n);
    st->imp.count = cx_max(st->imp.count, n);

    for (size_t i = start->as_int+len->as_int;
	 i < cx_min(prev_count, start->as_int+len->as_int+delta->as_int);
	 i++) {
      cx_box_deinit(cx_vec_get(&st->imp, i));
    }
  } else {
    for (size_t i = start->as_int+delta->as_int;
	 i < start->as_int;
	 i++) {
      cx_box_deinit(cx_vec_get(&st->imp, i));
    }
  }

  memmove(cx_vec_get(&st->imp, start->as_int+delta->as_int),
	  cx_vec_get(&st->imp, start->as_int),
	  len->as_int*sizeof(struct cx_box));

  if (delta->as_int > 0) {
    for (size_t i = start->as_int; i < start->as_int+delta->as_int; i++) {
      cx_box_init(cx_vec_get(&st->imp, i), s->cx->nil_type);
    }
  } else {
    for (size_t i = start->as_int+len->as_int+delta->as_int;
	 i < start->as_int+len->as_int;
	 i++) {
      cx_box_init(cx_vec_get(&st->imp, i), s->cx->nil_type);
    }
  }
  
  return true;
}

static bool riter_imp(struct cx_call *call) {
  struct cx_box *stv = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  struct cx_stack *st = stv->as_ptr;

  cx_box_init(cx_push(s), cx_type_get(s->cx->iter_type,
				      cx_type_arg(stv->type, 0)))->as_iter =
    cx_stack_iter_new(st, st->imp.count-1, -1, -1);
  
  return true;
}

static bool reset_imp(struct cx_call *call) {
  cx_reset(call->scope);
  return true;
}

static bool zap_imp(struct cx_call *call) {
  return true;
}

static bool copy_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_copy(cx_push(s), v);
  cx_copy(cx_push(s), v);
  return true;
}

static bool clone_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_copy(cx_push(s), v);
  cx_clone(cx_push(s), v);
  return true;
}

static bool swap_imp(struct cx_call *call) {
  struct cx_scope *s = call->scope;
  cx_copy(cx_push(s), cx_call_arg(call, 1));
  cx_copy(cx_push(s), cx_call_arg(call, 0));
  return true;
}

cx_lib(cx_init_stack, "cx/stack") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Cmp", "Int", "Opt", "Seq", "Stack", "Sym") ||
      !cx_use(cx, "cx/type", "new")) {
    return false;
  }

  cx_add_cfunc(lib, "len",
	       cx_args(cx_arg("s", cx->stack_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       len_imp);
  
  cx_add_cfunc(lib, "push",
	       cx_args(cx_arg("s", cx->stack_type),
		       cx_narg(cx, "val", 0, 0)),
	       cx_args(),
	       push_imp);

  cx_add_cfunc(lib, "pop",
	       cx_args(cx_arg("s", cx->stack_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx_arg_ref(cx, 0, 0)))),
	       pop_imp);

  cx_add_cfunc(lib, "get",
	       cx_args(cx_arg("s", cx->stack_type), cx_arg("i", cx->int_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx_arg_ref(cx, 0, 0)))),
	       get_imp);

  cx_add_cfunc(lib, "last",
	       cx_args(cx_arg("s", cx->stack_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx_arg_ref(cx, 0, 0)))),
	       last_imp);

  cx_add_cfunc(lib, "put",
	       cx_args(cx_arg("s", cx->stack_type),
		       cx_arg("i", cx->int_type),
		       cx_narg(cx, "val", 0, 0)),
	       cx_args(),
	       put_imp);

  cx_add_cfunc(lib, "get-rand",
	       cx_args(cx_arg("s", cx->stack_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx_arg_ref(cx, 0, 0)))),
	       get_rand_imp);

  cx_add_cfunc(lib, "stack",
	       cx_args(cx_arg("in", cx->seq_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->stack_type,
						cx_arg_ref(cx, 0, 0)))),
	       stack_imp);

  cx_add_cfunc(lib, "stash",
	       cx_args(),
	       cx_args(cx_arg(NULL, cx->stack_type)),
	       stash_imp);

  cx_add_cfunc(lib, "..",
	       cx_args(cx_arg("in", cx->seq_type)),
	       cx_args(),
	       splat_imp);

  cx_add_cfunc(lib, "clear",
	       cx_args(cx_arg("s", cx->stack_type)), cx_args(),
	       clear_imp);

  cx_add_cfunc(lib, "sort",
	       cx_args(cx_arg("s", cx->stack_type), cx_arg("cmp", cx->opt_type)),
	       cx_args(),
	       sort_imp);

  cx_add_cfunc(lib, "fill",
	       cx_args(cx_arg("s", cx->stack_type),
		       cx_arg("n", cx->int_type),
		       cx_arg("act", cx->opt_type)),
	       cx_args(),
	       fill_imp);

  cx_add_cfunc(lib, "move",
	       cx_args(cx_arg("s", cx->stack_type),
		       cx_arg("start", cx->int_type),
		       cx_arg("len", cx->int_type),
		       cx_arg("delta", cx->int_type)),
	       cx_args(),
	       move_imp);

  cx_add_cfunc(lib, "riter",
	       cx_args(cx_arg("s", cx->stack_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->iter_type,
						cx_arg_ref(cx, 0, 0)))),
	       riter_imp);

  cx_add_cfunc(lib, "|", cx_args(), cx_args(), reset_imp);
    
  cx_add_cfunc(lib, "_", cx_args(cx_arg("v", cx->opt_type)), cx_args(), zap_imp);
    
  cx_add_cfunc(lib, "%",
	       cx_args(cx_arg("v", cx->opt_type)), cx_args(cx_narg(cx, NULL, 0)),
	       copy_imp);
    
  cx_add_cfunc(lib, "%%",
	       cx_args(cx_arg("v", cx->opt_type)), cx_args(cx_narg(cx, NULL, 0)),
	       clone_imp);
    
  cx_add_cfunc(lib, "~",
	       cx_args(cx_arg("x", cx->opt_type), cx_arg("y", cx->opt_type)),
	       cx_args(cx_narg(cx, NULL, 1), cx_narg(cx, NULL, 0)),
	       swap_imp);

  return true;
}
