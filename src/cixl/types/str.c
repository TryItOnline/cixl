#include <inttypes.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/iter.h"
#include "cixl/types/str.h"

struct cx_str_iter {
  struct cx_iter iter;
  struct cx_str *str;
  char *ptr;
};

bool str_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  struct cx_str_iter *it = cx_baseof(iter, struct cx_str_iter, iter);
  char c = *it->ptr;
  
  if (c) {
    cx_box_init(out, scope->cx->char_type)->as_char = c;
    it->ptr++;
    return true;
  }

  iter->done = true;
  return false;
}

void *str_deinit(struct cx_iter *iter) {
  struct cx_str_iter *it = cx_baseof(iter, struct cx_str_iter, iter);
  cx_str_deref(it->str);
  return it;
}

cx_iter_type(str_iter, {
    type.next = str_next;
    type.deinit = str_deinit;
  });

struct cx_str_iter *cx_str_iter_new(struct cx_str *str) {
  struct cx_str_iter *it = malloc(sizeof(struct cx_str_iter));
  cx_iter_init(&it->iter, str_iter());
  it->str = cx_str_ref(str);
  it->ptr = str->data;
  return it;
}

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

void cx_str_deref(struct cx_str *str) {
  cx_test(str->nrefs);
  str->nrefs--;
  if (!str->nrefs) { free(str); }
}

static bool int_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box v = *cx_test(cx_pop(scope, false));
  struct cx_str *s = v.as_str;
  int64_t iv = strtoimax(s->data, NULL, 10);
  
  if (!iv && (!s->data[0] || s->data[0] != '0' || s->data[1])) {
    cx_box_init(cx_push(scope), cx->nil_type);
  } else {
    cx_box_init(cx_push(scope), cx->int_type)->as_int = iv;
  }
  
  cx_box_deinit(&v);
  return true;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_str == y->as_str;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  if (x->as_str->len != y->as_str->len) { return false; }
  return strcmp(x->as_str->data, y->as_str->data) == 0;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  return cx_cmp_str(&x->as_str->data, &y->as_str->data);
}

static bool ok_imp(struct cx_box *v) {
  return v->as_str->len;
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_str = cx_str_ref(src->as_str);
}

static struct cx_iter *iter_imp(struct cx_box *v) {
  return &cx_str_iter_new(v->as_str)->iter;
}

static void write_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "'%s'", v->as_str->data);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "'%s'@%d", v->as_str->data, v->as_str->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_str_deref(v->as_str);
}

struct cx_type *cx_init_str_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Str",
				  cx->any_type, cx->cmp_type, cx->seq_type);
  t->eqval = eqval_imp;
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->iter = iter_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;

  cx_add_cfunc(cx, "int", int_imp, cx_arg("s", t));

  return t;
}
