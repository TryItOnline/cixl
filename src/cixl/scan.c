#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scan.h"
#include "cixl/scope.h"
#include "cixl/types/func.h"
#include "cixl/vec.h"

struct cx_scan *cx_scan_init(struct cx_scan *scan,
			     struct cx_scope *scope,
			     struct cx_func *func,
			     cx_scan_callback_t callback,
			     void *data) {
  scan->scope = scope;
  scan->level = scope->cx->scan_level;
  scan->func = func;
  scan->callback = callback;
  scan->data = data;
  return scan;
}

void cx_scan(struct cx_scope *scope,
	     struct cx_func *func,
	     cx_scan_callback_t callback,
	     void *data) {  
  struct cx_scan *scan = cx_scan_init(cx_vec_push(&scope->cx->scans),
				      scope,
				      func,
				      callback,
				      data);

  if (scope->cuts.count) {
    struct cx_cut *c = cx_vec_peek(&scope->cuts, 0);
    if (!c->scan) { c->scan = scan; }
  }
}

bool cx_scan_ok(struct cx_scan *scan) {
  struct cx_scope *s = scan->scope;
  if (scan->level != s->cx->scan_level) { return false; }
  size_t cut_offs = 0;
  
  if (s->cuts.count) { 
    struct cx_cut *c = cx_vec_peek(&s->cuts, 0);
    if (c) { cut_offs = c->offs; }
  }
  
  return s->stack.count - cut_offs >= scan->func->nargs;
}

bool cx_scan_call(struct cx_scan *scan) {
  struct cx_scope *s = scan->scope;

  if (s->cuts.count) {
    struct cx_cut *c = cx_vec_peek(&s->cuts, 0);
    if (c->scan == scan) { cx_cut_deinit(cx_vec_pop(&s->cuts)); }
  }
  
  return scan->callback(scan, scan->data);
}
