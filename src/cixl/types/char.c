#include <ctype.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/scope.h"
#include "cixl/types/char.h"

static bool upper_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->char_type)->as_char = toupper(v.as_char);
  return true;
}

static bool lower_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->char_type)->as_char = tolower(v.as_char);
  return true;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_char == y->as_char;
}

static bool ok_imp(struct cx_box *v) {
  return v->as_char;
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_char = src->as_char;
}

static void fprint_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "\\%c", v->as_char);
}

struct cx_type *cx_init_char_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Char", cx->any_type);
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->fprint = fprint_imp;
  
  cx_add_func(cx, "upper", cx_arg(t))->ptr = upper_imp;
  cx_add_func(cx, "lower", cx_arg(t))->ptr = lower_imp;

  return t;
}
