#include <stdbool.h>
#include <stdlib.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/iter.h"
#include "cixl/malloc.h"
#include "cixl/stack.h"

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
  v->imp.alloc = &cx->stack_items_alloc;
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

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  struct cx_stack *xv = x->as_ptr, *yv = y->as_ptr;
  if (xv->imp.count != yv->imp.count) { return false; }
  
  for (size_t i = 0; i < xv->imp.count; i++) {
    if (!cx_eqval(cx_vec_get(&xv->imp, i), cx_vec_get(&yv->imp, i))) {
      return false;
    }
  }
  
  return true;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
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

static bool ok_imp(struct cx_box *b) {
  struct cx_stack *v = b->as_ptr;
  return v->imp.count;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_ptr = cx_stack_ref(src->as_ptr);
}

static void clone_imp(struct cx_box *dst, struct cx_box *src) {
  struct cx *cx = src->type->lib->cx;
  struct cx_stack *src_stack = src->as_ptr, *dst_stack = cx_stack_new(cx);
  dst->as_ptr = dst_stack;

  cx_do_vec(&src_stack->imp, struct cx_box, v) {
    cx_clone(cx_vec_push(&dst_stack->imp), v);
  }
}

static struct cx_iter *iter_imp(struct cx_box *v) {
  return &cx_stack_iter_new(v->as_ptr)->iter;
}

static void write_imp(struct cx_box *b, FILE *out) {
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

static void dump_imp(struct cx_box *b, FILE *out) {
  struct cx_stack *v = b->as_ptr;
  cx_stack_dump(&v->imp, out);
  fprintf(out, "r%d", v->nrefs);
}

static void print_imp(struct cx_box *b, FILE *out) {
  struct cx_stack *stack = b->as_ptr;
  cx_do_vec(&stack->imp, struct cx_box, v) {
    cx_print(v, out);
  }
}

static void deinit_imp(struct cx_box *v) {
  cx_stack_deref(v->as_ptr);
}


struct cx_type *cx_init_stack_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Stack", cx->cmp_type, cx->seq_type);
  t->eqval = eqval_imp;
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->clone = clone_imp;
  t->iter = iter_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->print = print_imp;
  t->deinit = deinit_imp;
  return t;
}
