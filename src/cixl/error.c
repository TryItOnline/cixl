#include <stdarg.h>
#include <string.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/catch.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/scope.h"
#include "cixl/stack.h"
#include "cixl/str.h"
#include "cixl/util.h"

struct cx_error *cx_error_init(struct cx_error *e,
			       struct cx *cx,
			       int row, int col,
			       struct cx_box *v) {
  e->row = row;
  e->col = col;
  e->nrefs = 1;  
  cx_copy(&e->value, v);

  struct cx_scope *s = cx_scope(cx, 0);

  cx_vec_init(&e->stack, sizeof(struct cx_box));
  cx_vec_grow(&e->stack, s->stack.count);
  cx_do_vec(&s->stack, struct cx_box, v) { cx_copy(cx_vec_push(&e->stack), v); }

  cx_vec_init(&e->calls, sizeof(struct cx_call));

  if (cx->calls.count > 1) {
    size_t n = cx->calls.count-1;
    cx_vec_grow(&e->calls, n);
    memcpy(e->calls.items, cx->calls.items, n*sizeof(struct cx_call));
    e->calls.count = n;
  }

  return e;
}

struct cx_error *cx_error_deinit(struct cx_error *e) {
  cx_do_vec(&e->stack, struct cx_box, v) { cx_box_deinit(v); }
  cx_vec_deinit(&e->stack);
  cx_vec_deinit(&e->calls);
  cx_box_deinit(&e->value);
  return e;
}

struct cx_error *cx_error_ref(struct cx_error *e) {
  e->nrefs++;
  return e;
}

void cx_error_deref(struct cx_error *e) {
  cx_test(e->nrefs);
  e->nrefs--;
  if (!e->nrefs) { free(cx_error_deinit(e)); }
}

void cx_error_dump(struct cx_error *e, FILE *out) {
  cx_do_vec(&e->calls, struct cx_call, c) {
    fprintf(out, "While calling %s<%s> from row %d, col %d\n",
	    c->target->func->id, c->target->id,
	    c->row, c->col);
  }
  
  fprintf(out, "Error in row %d, col %d:\n", e->row, e->col);
  cx_print(&e->value, out);
  fputc('\n', out);
  cx_stack_dump(&e->stack, out);
  fputs("\n\n", out);
}

struct cx_error *new_error(struct cx *cx, int row, int col, struct cx_box *v) {
  return cx_error_init(cx_vec_push(&cx->errors), cx, row, col, v);
}

struct cx_error *cx_error(struct cx *cx, int row, int col, const char *spec, ...) {
  va_list args;
  va_start(args, spec);
  char *msg = cx_vfmt(spec, args);
  va_end(args);
  
  struct cx_box v;
  cx_box_init(&v, cx->str_type)->as_str = cx_str_new(msg, strlen(msg));
  free(msg);
  
  struct cx_error *e = new_error(cx, row, col, &v);
  cx_box_deinit(&v);
  return e;
}

struct cx_error *cx_throw(struct cx *cx, struct cx_box *v) {
  return new_error(cx, cx->row, cx->col, v);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_error == y->as_error;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_error = cx_error_ref(src->as_error);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "Error(");
  cx_dump(&v->as_error->value, out);
  fprintf(out, ")r%d", v->as_error->nrefs);
}

static void print_imp(struct cx_box *v, FILE *out) {
  cx_error_dump(v->as_error, out);
}

static void deinit_imp(struct cx_box *v) {
  cx_error_deref(v->as_error);
}

struct cx_type *cx_init_error_type(struct cx_lib *lib) {
    struct cx_type *t = cx_add_type(lib, "Error", lib->cx->any_type);
    
    t->equid = equid_imp;
    t->copy = copy_imp;
    t->dump = dump_imp;
    t->print = print_imp;
    t->deinit = deinit_imp;
    return t;
}
