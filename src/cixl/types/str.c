#include <inttypes.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/str.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

static bool int_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box v = *cx_test(cx_pop(scope, false));
  char *s = v.as_ptr;
  cx_int_t iv = strtoimax(s, NULL, 10);
  bool ok = false;
  
  if (!iv && (!s[0] || s[0] != '0' || s[1])) {
    cx_error(cx, cx->row, cx->col, "Failed parsing int: '%s'", s);
    goto exit;
  }
  
  cx_box_init(cx_push(scope), cx->int_type)->as_int = iv;
  ok = true;
 exit:
  cx_box_deinit(&v);
  return ok;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y, struct cx_scope *scope) {
  return strcmp(x->as_ptr, y->as_ptr) == 0;
}

static enum cx_cmp cmp_imp(struct cx_box *x, struct cx_box *y) {
  return cx_cmp_str(&x, &y);
}

static bool ok_imp(struct cx_box *v) {
  char *s = v->as_ptr;
  return s[0];
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_ptr = strdup(src->as_ptr);
}

static void fprint_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "'%s'", (char *)v->as_ptr);
}

static void deinit_imp(struct cx_box *v) {
  free(v->as_ptr);
}

struct cx_type *cx_init_str_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Str", cx->any_type);
  t->eqval = eqval_imp;
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->fprint = fprint_imp;
  t->deinit = deinit_imp;

  cx_add_func(cx, "int", cx_arg(t))->ptr = int_imp;

  return t;
}
