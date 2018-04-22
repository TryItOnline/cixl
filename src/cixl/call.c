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
  return c;
}

struct cx_call *cx_call_deinit(struct cx_call *c) {
  cx_call_deinit_args(c);
  cx_scope_deref(c->scope);
  return c;
}

struct cx_box *cx_call_arg(struct cx_call *c, unsigned int i) {  
  if (i > c->fimp->func->nargs) {
    struct cx *cx = c->scope->cx;

    cx_error(cx, cx->row, cx->col,
	     "Arg index out of bounds for %s: %d",
	     c->fimp->func->id, i);
    
    return NULL;
  }

  return c->args+i;
}

bool cx_call_pop_args(struct cx_call *c) {
  struct cx *cx = c->scope->cx;
  struct cx_vec *s = &c->scope->stack;
  int nargs = c->fimp->func->nargs;
  if (!nargs) { return true; }  
  
  if (s->count < nargs) {
    cx_error(cx, cx->row, cx->col, "Not enough args to call %s", c->fimp->func->id);
    return false;
  }

  memcpy(c->args,
	 cx_vec_peek(s, nargs-1),
	 nargs*sizeof(struct cx_box));
  
  s->count -= nargs;
  return true;
}

void cx_call_deinit_args(struct cx_call *c) {
  for (unsigned int i=0; i < c->fimp->func->nargs; i++) { cx_box_deinit(c->args+i); }
}
