#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/lib.h"
#include "cixl/libs/stack.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/iter.h"

struct cx_stack_iter {
  struct cx_iter iter;
  struct cx_stack *stack;
  size_t i;
};

bool stack_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  struct cx_stack_iter *it = cx_baseof(iter, struct cx_stack_iter, iter);

  if (it->i < it->stack->imp.count) {
    cx_copy(out, cx_vec_get(&it->stack->imp, it->i));
    it->i++;
    return true;
  }

  iter->done = true;
  return false;
}

void *stack_deinit(struct cx_iter *iter) {
  struct cx_stack_iter *it = cx_baseof(iter, struct cx_stack_iter, iter);
  cx_stack_deref(it->stack);
  return it;
}

cx_iter_type(stack_iter, {
    type.next = stack_next;
    type.deinit = stack_deinit;
  });

struct cx_stack_iter *cx_stack_iter_new(struct cx_stack *stack) {
  struct cx_stack_iter *it = malloc(sizeof(struct cx_stack_iter));
  cx_iter_init(&it->iter, stack_iter());
  it->stack = cx_stack_ref(stack);
  it->i = 0;
  return it;
}

struct cx_stack *cx_stack_new(struct cx *cx) {
  struct cx_stack *v = cx_malloc(&cx->stack_alloc);
  v->cx = cx;
  cx_vec_init(&v->imp, sizeof(struct cx_box));
  v->nrefs = 1;
  return v;
}

struct cx_stack *cx_stack_ref(struct cx_stack *stack) {
  stack->nrefs++;
  return stack;
}

void cx_stack_deref(struct cx_stack *stack) {
  cx_test(stack->nrefs);
  stack->nrefs--;

  if (!stack->nrefs) {
    cx_do_vec(&stack->imp, struct cx_box, b) { cx_box_deinit(b); }
    cx_vec_deinit(&stack->imp);
    cx_free(&stack->cx->stack_alloc, stack);
  }
}

void cx_stack_dump(struct cx_vec *imp, FILE *out) {
  fputc('[', out);
  char sep = 0;
  
  cx_do_vec(imp, struct cx_box, b) {
    if (sep) { fputc(sep, out); }
    cx_dump(b, out);
    sep = ' ';
  }

  fputc(']', out);
}

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

cx_lib(cx_init_stack, "cx/stack", {
    if (!cx_use(cx, "cx/stack/types", false)) { return false; }

    cx_add_cfunc(cx, "len",
		 cx_args(cx_arg("vec", cx->stack_type)),
		 cx_args(cx_arg(NULL, cx->int_type)),
		 len_imp);
  
    cx_add_cfunc(cx, "push",
		 cx_args(cx_arg("vec", cx->stack_type), cx_arg("val", cx->any_type)),
		 cx_args(),
		 push_imp);

    cx_add_cfunc(cx, "pop",
		 cx_args(cx_arg("vec", cx->stack_type)),
		 cx_args(cx_arg(NULL, cx->opt_type)),
		 pop_imp);

    cx_add_cfunc(cx, "stack",
		 cx_args(cx_arg("in", cx->seq_type)),
		 cx_args(cx_arg(NULL, cx->stack_type)),
		 seq_imp);

    cx_add_cfunc(cx, "clear",
		 cx_args(cx_arg("vec", cx->stack_type)), cx_args(),
		 clear_imp);

    cx_add_cfunc(cx, "sort",
		 cx_args(cx_arg("vec", cx->stack_type), cx_arg("cmp", cx->opt_type)),
		 cx_args(),
		 sort_imp);

    return true;
  })

static bool stack_equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool stack_eqval_imp(struct cx_box *x, struct cx_box *y) {
  struct cx_stack *xv = x->as_ptr, *yv = y->as_ptr;
  if (xv->imp.count != yv->imp.count) { return false; }
  
  for (size_t i = 0; i < xv->imp.count; i++) {
    if (!cx_eqval(cx_vec_get(&xv->imp, i), cx_vec_get(&yv->imp, i))) {
      return false;
    }
  }
  
  return true;
}

static enum cx_cmp stack_cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  struct cx_stack *xv = x->as_ptr, *yv = y->as_ptr;
  struct cx_box *xe = cx_vec_end(&xv->imp), *ye = cx_vec_end(&yv->imp);
  
  for (struct cx_box *xp = cx_vec_start(&xv->imp), *yp = cx_vec_start(&yv->imp);
       xp != xe && yp != ye;
       xp++, yp++) {
    enum cx_cmp res = cx_cmp(xp, yp);
    if (res != CX_CMP_EQ) { return res; }
  }

  if (xv->imp.count < yv->imp.count) { return CX_CMP_LT; }
  return (xv->imp.count > yv->imp.count) ? CX_CMP_GT : CX_CMP_EQ;
}

static bool stack_ok_imp(struct cx_box *b) {
  struct cx_stack *v = b->as_ptr;
  return v->imp.count;
}

static void stack_copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_ptr = cx_stack_ref(src->as_ptr);
}

static void stack_clone_imp(struct cx_box *dst, struct cx_box *src) {
  struct cx *cx = src->type->cx;
  struct cx_stack *src_stack = src->as_ptr, *dst_stack = cx_stack_new(cx);
  dst->as_ptr = dst_stack;

  cx_do_vec(&src_stack->imp, struct cx_box, v) {
    cx_clone(cx_vec_push(&dst_stack->imp), v);
  }
}

static struct cx_iter *stack_iter_imp(struct cx_box *v) {
  return &cx_stack_iter_new(v->as_ptr)->iter;
}

static void stack_write_imp(struct cx_box *b, FILE *out) {
  struct cx_stack *v = b->as_ptr;

  fputs("([", out);
  char sep = 0;
  
  cx_do_vec(&v->imp, struct cx_box, b) {
    if (sep) { fputc(sep, out); }
    cx_write(b, out);
    sep = ' ';
  }

  fputs("])", out);
}

static void stack_dump_imp(struct cx_box *b, FILE *out) {
  struct cx_stack *v = b->as_ptr;
  cx_stack_dump(&v->imp, out);
  fprintf(out, "r%d", v->nrefs);
}

static void stack_print_imp(struct cx_box *b, FILE *out) {
  struct cx_stack *stack = b->as_ptr;
  cx_do_vec(&stack->imp, struct cx_box, v) {
    cx_print(v, out);
  }
}

static void stack_deinit_imp(struct cx_box *v) {
  cx_stack_deref(v->as_ptr);
}

cx_lib(cx_init_stack_types, "cx/stack/types", {
    struct cx_type *t = cx_add_type(cx, "Stack", cx->cmp_type, cx->seq_type);
    t->eqval = stack_eqval_imp;
    t->equid = stack_equid_imp;
    t->cmp = stack_cmp_imp;
    t->ok = stack_ok_imp;
    t->copy = stack_copy_imp;
    t->clone = stack_clone_imp;
    t->iter = stack_iter_imp;
    t->write = stack_write_imp;
    t->dump = stack_dump_imp;
    t->print = stack_print_imp;
    t->deinit = stack_deinit_imp;
    
    cx->stack_type = t;
    return true;
  })

static bool reset_imp(struct cx_scope *scope) {
  cx_do_vec(&scope->stack, struct cx_box, v) { cx_box_deinit(v); }
  cx_vec_clear(&scope->stack);
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

cx_lib(cx_init_stack_ops, "cx/stack/ops", {
    cx_add_cfunc(cx, "|", cx_args(), cx_args(), reset_imp);
    
    cx_add_cfunc(cx, "_", cx_args(), cx_args(), zap_imp);
    
    cx_add_cfunc(cx, "%",
		 cx_args(cx_arg("v", cx->opt_type)), cx_args(cx_narg(NULL, 0)),
		 copy_imp);
    
    cx_add_cfunc(cx, "%%",
		 cx_args(cx_arg("v", cx->opt_type)), cx_args(cx_narg(NULL, 0)),
		 clone_imp);
    
    cx_add_cfunc(cx, "~",
		 cx_args(cx_arg("x", cx->opt_type), cx_arg("y", cx->opt_type)),
		 cx_args(cx_narg(NULL, 1), cx_narg(NULL, 0)),
		 flip_imp);

    return true;
  })
