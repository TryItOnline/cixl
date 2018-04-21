#include <string.h>

#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/scope.h"

struct cx_call *cx_call_init(struct cx_call *c,
			     int row, int col,
			     struct cx_fimp *fimp,
			     struct cx_scope *scope) {
  c->row = row;
  c->col = col;
  c->fimp = fimp;
  c->scope = cx_scope_ref(scope);
  c->recalls = 0;
  cx_vec_init(&c->args, sizeof(struct cx_box));
  c->args.alloc = &scope->cx->stack_items_alloc;
  return c;
}

static void clear_args(struct cx_call *c) {
  cx_do_vec(&c->args, struct cx_box, v) { cx_box_deinit(v); }
  cx_vec_clear(&c->args);
}

struct cx_call *cx_call_deinit(struct cx_call *c) {
  clear_args(c);
  cx_scope_deref(c->scope);
  cx_vec_deinit(&c->args);
  return c;
}

struct cx_box *cx_call_arg(struct cx_call *c, int i) {  
  if (i < 0 || i > c->args.count) {
    struct cx *cx = c->scope->cx;

    cx_error(cx, cx->row, cx->col,
	     "Arg index out of bounds for %s: %d",
	     c->fimp->func->id, i);
    
    return NULL;
  }

  return cx_vec_get(&c->args, i);
}

bool cx_call_pop_args(struct cx_call *c) {
  struct cx *cx = c->scope->cx;
  struct cx_vec *s = &c->scope->stack;
  int nargs = c->fimp->func->nargs;
  
  if (s->count < nargs) {
    cx_error(cx, cx->row, cx->col, "Not enough args to call %s", c->fimp->func->id);
    return false;
  }

  clear_args(c);
  if (!nargs) { return true; }  
  cx_vec_grow(&c->args, nargs);

  memcpy(c->args.items,
	 cx_vec_peek(s, nargs-1),
	 nargs*sizeof(struct cx_box));
  
  s->count -= nargs;
  c->args.count = nargs;
  return true;
}
