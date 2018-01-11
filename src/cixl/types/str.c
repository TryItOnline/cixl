#include <inttypes.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/str.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

struct cx_str *cx_str_new(const char *data) {
  size_t len = strlen(data);
  struct cx_str *str = malloc(sizeof(struct cx_str)+len+1);
  strcpy(str->data, data);
  str->len = len;
  str->nrefs = 1;
  return str;
}

struct cx_str *cx_str_ref(struct cx_str *str) {
  str->nrefs++;
  return str;
}

void cx_str_unref(struct cx_str *str) {
  cx_test(str->nrefs > 0);
  str->nrefs--;
  if (!str->nrefs) { free(str); }
}

static bool int_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box v = *cx_test(cx_pop(scope, false));
  struct cx_str *s = v.as_str;
  int64_t iv = strtoimax(s->data, NULL, 10);
  bool ok = false;
  
  if (!iv && (!s->data[0] || s->data[0] != '0' || s->data[1])) {
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
  return x->as_str == y->as_str;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  if (x->as_str->len != y->as_str->len) { return false; }
  return strcmp(x->as_str->data, y->as_str->data) == 0;
}

static enum cx_cmp cmp_imp(struct cx_box *x, struct cx_box *y) {
  return cx_cmp_str(&x->as_str->data, &y->as_str->data);
}

static bool ok_imp(struct cx_box *v) {
  return v->as_str->len;
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_str = cx_str_ref(src->as_str);
}

static void fprint_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "'%s'@%d", v->as_str->data, v->as_str->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_str_unref(v->as_str);
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
