#include <stdarg.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/util.h"

struct cx_error *cx_error_init(struct cx_error *e, int row, int col, char *msg) {
  e->row = row;
  e->col = col;
  e->msg = msg;
  cx_vec_init(&e->stack, sizeof(struct cx_box));
  return e;
}

struct cx_error *cx_error_deinit(struct cx_error *e) {
  cx_do_vec(&e->stack, struct cx_box, v) { cx_box_deinit(v); }
  cx_vec_deinit(&e->stack);
  free(e->msg);
  return e;
}

struct cx_error *cx_error(struct cx *cx, int row, int col, const char *spec, ...) {
  va_list args;
  va_start(args, spec);
  char *msg = cx_vfmt(spec, args);
  va_end(args);

  struct cx_error *e = cx_error_init(cx_vec_push(&cx->errors), row, col, msg);
  struct cx_scope *s = cx_scope(cx, 0);
  cx_vec_grow(&e->stack, s->stack.count);
  cx_do_vec(&s->stack, struct cx_box, v) { cx_copy(cx_vec_push(&e->stack), v); }
  return e;
}
