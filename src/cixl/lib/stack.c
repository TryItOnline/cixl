#include <inttypes.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/iter.h"
#include "cixl/lib.h"
#include "cixl/lib/stack.h"
#include "cixl/scope.h"
#include "cixl/stack.h"

static bool len_imp(struct cx_scope *scope) {
  struct cx_box vec = *cx_test(cx_pop(scope, false));
  struct cx_stack *v = vec.as_ptr;
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = v->imp.count;
  cx_box_deinit(&vec);
  return true;
}

static bool push_imp(struct cx_scope *scope) {
  struct cx_box
    val = *cx_test(cx_pop(scope, false)),
    vec = *cx_test(cx_pop(scope, false));
  
  struct cx_stack *v = vec.as_ptr;
  *(struct cx_box *)cx_vec_push(&v->imp) = val;
  cx_box_deinit(&vec);
  return true;
}

static bool put_imp(struct cx_scope *scope) {
  struct cx_box
    val = *cx_test(cx_pop(scope, false)),
    i = *cx_test(cx_pop(scope, false)),
    sv = *cx_test(cx_pop(scope, false));
  
  struct cx_stack *s = sv.as_ptr;
  struct cx *cx = scope->cx;
  bool ok = false;
  
  if (i.as_int < 0 || i.as_int >= s->imp.count) {
    cx_error(cx, cx->row, cx->col, "Index out of bounds: %" PRId64, i.as_int);
    cx_box_deinit(&val);
    goto exit;
  }

  struct cx_box *p = cx_vec_get(&s->imp, i.as_int);
  cx_box_deinit(p);
  *p = val;
  ok = true;
 exit:
  cx_box_deinit(&sv);
  return ok;
}

static bool pop_imp(struct cx_scope *scope) {
  struct cx_box vec = *cx_test(cx_pop(scope, false));
  struct cx_stack *v = vec.as_ptr;

  if (v->imp.count) {
    *cx_push(scope) = *(struct cx_box *)cx_vec_pop(&v->imp);
  } else {
    cx_box_init(cx_push(scope), scope->cx->nil_type);
  }
  
  cx_box_deinit(&vec);
  return true;
}

static bool get_imp(struct cx_scope *scope) {
  struct cx_box
    i = *cx_test(cx_pop(scope, false)),
    v = *cx_test(cx_pop(scope, false));
  
  struct cx_stack *s = v.as_ptr;
  
  if (i.as_int < s->imp.count) {
    cx_copy(cx_push(scope), (struct cx_box *)cx_vec_get(&s->imp, i.as_int));
  } else {
    cx_box_init(cx_push(scope), scope->cx->nil_type);
  }
  
  cx_box_deinit(&i);
  cx_box_deinit(&v);
  return true;
}

static bool get_rand_imp(struct cx_scope *scope) {
  struct cx_box sv = *cx_test(cx_pop(scope, false));
  struct cx_stack *s = sv.as_ptr;
  
  if (s->imp.count) {
    int64_t i = cx_rand(s->imp.count);
    cx_copy(cx_push(scope), (struct cx_box *)cx_vec_get(&s->imp, i));
  } else {
    cx_box_init(cx_push(scope), scope->cx->nil_type);
  }
  
  cx_box_deinit(&sv);
  return true;
}

static bool seq_imp(struct cx_scope *scope) {
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_iter *it = cx_iter(&in);
  struct cx_stack *out = cx_stack_new(scope->cx);
  struct cx_box v;
  
  while (cx_iter_next(it, &v, scope)) {
    *(struct cx_box *)cx_vec_push(&out->imp) = v;
  }

  cx_box_init(cx_push(scope), scope->cx->stack_type)->as_ptr = out;
  cx_box_deinit(&in);
  cx_iter_deref(it);
  return true;
}

static bool stash_imp(struct cx_scope *scope) {
  cx_stash(scope);
  return true;
}

static bool splat_imp(struct cx_scope *scope) {
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_stack *s = in.as_ptr;
  cx_vec_grow(&scope->stack, scope->stack.count+s->imp.count);
  cx_do_vec(&s->imp, struct cx_box, v) { cx_copy(cx_vec_push(&scope->stack), v); }
  cx_box_deinit(&in);
  return true;
}

static bool clear_imp(struct cx_scope *scope) {
  struct cx_box vec = *cx_test(cx_pop(scope, false));
  struct cx_stack *v = vec.as_ptr;
  cx_vec_clear(&v->imp);
  cx_box_deinit(&vec);
  return true;
}

static bool sort_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    cmp = *cx_test(cx_pop(scope, false)),
    vec = *cx_test(cx_pop(scope, false));

  struct cx_stack *v = vec.as_ptr;
  struct cx_sym lt = cx_sym(cx, "<"), gt = cx_sym(cx, ">");
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

static bool fill_imp(struct cx_scope *scope) {
  struct cx_box
    act = *cx_test(cx_pop(scope, false)),
    n = *cx_test(cx_pop(scope, false)),
    in = *cx_test(cx_pop(scope, false));
    
  struct cx_stack *s = in.as_ptr;
  cx_vec_grow(&s->imp, s->imp.count+n.as_int);
  bool ok = false;
  
  for (int64_t i=0; i<n.as_int; i++) {
    if (!cx_call(&act, scope)) { goto exit; }
    struct cx_box *v = cx_pop(scope, false);
    if (!v) { goto exit; }
    *(struct cx_box *)cx_vec_push(&s->imp) = *v;
  }

  ok = true;
 exit:
  cx_box_deinit(&act);
  cx_box_deinit(&in);
  return ok;
}

static bool move_imp(struct cx_scope *scope) {
  struct cx_box
    delta = *cx_test(cx_pop(scope, false)),
    len = *cx_test(cx_pop(scope, false)),
    start = *cx_test(cx_pop(scope, false)),
    in = *cx_test(cx_pop(scope, false));
    
  struct cx_stack *s = in.as_ptr;
  struct cx *cx = scope->cx;
  bool ok = false;

  if (start.as_int+delta.as_int < 0 || start.as_int+len.as_int > s->imp.count) {
    cx_error(cx, cx->row, cx->col, "Move out of bounds");
    goto exit;
  }

  size_t prev_count = s->imp.count;
  
  if (delta.as_int > 0) {
    size_t n = start.as_int+len.as_int+delta.as_int;
    cx_vec_grow(&s->imp, n);
    s->imp.count = cx_max(s->imp.count, n);

    for (size_t i = start.as_int+len.as_int;
	 i < cx_min(prev_count, start.as_int+len.as_int+delta.as_int);
	 i++) {
      cx_box_deinit(cx_vec_get(&s->imp, i));
    }
  } else {
    for (size_t i = start.as_int+delta.as_int;
	 i < start.as_int;
	 i++) {
      cx_box_deinit(cx_vec_get(&s->imp, i));
    }
  }

  memmove(cx_vec_get(&s->imp, start.as_int+delta.as_int),
	  cx_vec_get(&s->imp, start.as_int),
	  len.as_int*sizeof(struct cx_box));

  if (delta.as_int > 0) {
    for (size_t i = start.as_int; i < start.as_int+delta.as_int; i++) {
      cx_box_init(cx_vec_get(&s->imp, i), cx->nil_type);
    }
  } else {
    for (size_t i = start.as_int+len.as_int+delta.as_int;
	 i < start.as_int+len.as_int;
	 i++) {
      cx_box_init(cx_vec_get(&s->imp, i), cx->nil_type);
    }
  }
  
  ok = true;
 exit:
  cx_box_deinit(&in);
  return ok;
}

static bool reset_imp(struct cx_scope *scope) {
  cx_reset(scope);
  return true;
}

static bool zap_imp(struct cx_scope *scope) {
  cx_box_deinit(cx_test(cx_pop(scope, false)));
  return true;
}

static bool copy_imp(struct cx_scope *scope) {
  cx_copy(cx_push(scope), cx_test(cx_peek(scope, true)));
  return true;
}

static bool clone_imp(struct cx_scope *scope) {
  cx_clone(cx_push(scope), cx_test(cx_peek(scope, true)));
  return true;
}

static bool flip_imp(struct cx_scope *scope) {
  if (scope->stack.count < 2) {
    struct cx *cx = scope->cx;
    cx_error(cx, cx->row, cx->col, "Nothing to flip");
    return false;
  }

  struct cx_box *ptr = cx_vec_peek(&scope->stack, 0), tmp = *ptr;
  *ptr = *(ptr-1);
  *(ptr-1) = tmp;
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
	       cx_args(cx_arg("s", cx->stack_type), cx_arg("val", cx->opt_type)),
	       cx_args(),
	       push_imp);

  cx_add_cfunc(lib, "pop",
	       cx_args(cx_arg("s", cx->stack_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       pop_imp);

  cx_add_cfunc(lib, "get",
	       cx_args(cx_arg("s", cx->stack_type), cx_arg("i", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       get_imp);

  cx_add_cfunc(lib, "put",
	       cx_args(cx_arg("s", cx->stack_type),
		       cx_arg("i", cx->int_type),
		       cx_arg("val", cx->opt_type)),
	       cx_args(),
	       put_imp);

  cx_add_cfunc(lib, "get-rand",
	       cx_args(cx_arg("s", cx->stack_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       get_rand_imp);

  cx_add_cfunc(lib, "stack",
	       cx_args(cx_arg("in", cx->seq_type)),
	       cx_args(cx_arg(NULL, cx->stack_type)),
	       seq_imp);

  cx_add_cfunc(lib, "stash",
	       cx_args(),
	       cx_args(cx_arg(NULL, cx->stack_type)),
	       stash_imp);

  cx_add_cfunc(lib, "splat",
	       cx_args(cx_arg("in", cx->stack_type)),
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
		       cx_arg("delta", cx->opt_type)),
	       cx_args(),
	       move_imp);

  cx_add_cfunc(lib, "|", cx_args(), cx_args(), reset_imp);
    
  cx_add_cfunc(lib, "_", cx_args(), cx_args(), zap_imp);
    
  cx_add_cfunc(lib, "%",
	       cx_args(cx_arg("v", cx->opt_type)), cx_args(cx_narg(NULL, 0)),
	       copy_imp);
    
  cx_add_cfunc(lib, "%%",
	       cx_args(cx_arg("v", cx->opt_type)), cx_args(cx_narg(NULL, 0)),
	       clone_imp);
    
  cx_add_cfunc(lib, "~",
	       cx_args(cx_arg("x", cx->opt_type), cx_arg("y", cx->opt_type)),
	       cx_args(cx_narg(NULL, 1), cx_narg(NULL, 0)),
	       flip_imp);

  return true;
}
