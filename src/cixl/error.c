#include <stdarg.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/catch.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/util.h"

struct cx_error *cx_error_init(struct cx_error *e,
			       struct cx *cx,
			       int row, int col,
			       struct cx_box *v) {
  e->row = row;
  e->col = col;
  e->nrefs = 1;
  
  cx_vec_init(&e->stack, sizeof(struct cx_box));
  cx_copy(&e->value, v);

  struct cx_scope *s = cx_scope(cx, 0);
  cx_vec_grow(&e->stack, s->stack.count);
  cx_do_vec(&s->stack, struct cx_box, v) { cx_copy(cx_vec_push(&e->stack), v); }

  return e;
}

struct cx_error *cx_error_deinit(struct cx_error *e) {
  cx_do_vec(&e->stack, struct cx_box, v) { cx_box_deinit(v); }
  cx_vec_deinit(&e->stack);
  cx_box_deinit(&e->value);
  return e;
}

struct cx_error *new_error(struct cx *cx, int row, int col, struct cx_box *v) {
  while (true) {
    struct cx_scope *s = cx_scope(cx, 0);

    if (s->catches.count) {
      for (struct cx_catch *c = cx_vec_peek(&s->catches, 0);
	   s->catches.count;
	   c--, s->catches.count--) {
	if (cx_is(v->type, c->type)) {
	  s->catches.count--;
	  
	  for (struct cx_catch *cc = c-1;
	       s->catches.count && cc->tok_idx == c->tok_idx;
	       cx_catch_deinit(cc), s->catches.count--, cc--);
	  
	  cx_copy(cx_push(s), v);
	  cx_catch_eval(c);
	  cx_catch_deinit(c);
	  return NULL;	  
	}

	cx_catch_deinit(c);
      }
    }
    
    if (s == cx->root_scope) { break; }
    cx_end(cx);
  }
  
  return cx_error_init(cx_vec_push(&cx->errors), cx, row, col, v);
}

struct cx_error *cx_error(struct cx *cx, int row, int col, const char *spec, ...) {
  va_list args;
  va_start(args, spec);
  char *msg = cx_vfmt(spec, args);
  va_end(args);
  
  struct cx_box v;
  cx_box_init(&v, cx->str_type)->as_str = cx_str_new(msg);
  free(msg);
  
  struct cx_error *e = new_error(cx, row, col, &v);
  cx_box_deinit(&v);
  return e;
}

struct cx_error *cx_throw(struct cx *cx, struct cx_box *v) {
  return new_error(cx, cx->row, cx->col, v);
}
