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
  struct cx_error *e = cx_error_init(cx_vec_push(&cx->throwing), cx, row, col, v);

  while (true) {
    struct cx_scope *s = cx_scope(cx, 0);
      
    if (s->catches.count) {  
      for (struct cx_catch *c = cx_vec_peek(&s->catches, 0);
	   s->catches.count;
	   c--, s->catches.count--) {	
	if (c->type == cx->nil_type) {
	  cx_catch_eval(c);
	} else if (cx_is(v->type, c->type)) {
	  s->catches.count--;

	  for (struct cx_catch *cc = c-1;
	       s->catches.count && cc->tok_idx == c->tok_idx;
	       cx_catch_deinit(cc), s->catches.count--, cc--) {
	    if (cc->type == cx->nil_type) { cx_catch_eval(cc); }
	  }
	  
	  cx_copy(cx_push(s), v);
	  cx_catch_eval(c);
	  cx_catch_deinit(c);
	  cx_error_deinit(e);
	  cx_vec_pop(&cx->throwing);
	  return NULL;	  
	}

	cx_catch_deinit(c);
      }
    }
    
    if (s == cx->root_scope) { break; }
    cx_end(cx);
  }

  struct cx_error *ep = cx_vec_push(&cx->errors);
  cx_vec_pop(&cx->throwing);
  *ep = *e;
  return ep;
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
