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

struct char_iter {
  struct cx_iter iter;
  struct cx_str *str;
  char *ptr;
};

static bool char_next(struct cx_iter *iter,
		      struct cx_box *out,
		      struct cx_scope *scope) {
  struct char_iter *it = cx_baseof(iter, struct char_iter, iter);
  unsigned char c = *it->ptr;
  
  if (c) {
    cx_box_init(out, scope->cx->char_type)->as_char = c;
    it->ptr++;
    return true;
  }

  iter->done = true;
  return false;
}

static void *char_deinit(struct cx_iter *iter) {
  struct char_iter *it = cx_baseof(iter, struct char_iter, iter);
  cx_str_deref(it->str);
  return it;
}

static cx_iter_type(char_iter, {
    type.next = char_next;
    type.deinit = char_deinit;
  });

static struct cx_iter *char_iter_new(struct cx_str *str) {
  struct char_iter *it = malloc(sizeof(struct char_iter));
  cx_iter_init(&it->iter, char_iter());
  it->str = cx_str_ref(str);
  it->ptr = str->data;
  return &it->iter;
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

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_str == y->as_str;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  if (x->as_str->len != y->as_str->len) { return false; }
  return strcmp(x->as_str->data, y->as_str->data) == 0;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  const char *xs = x->as_str->data, *ys = y->as_str->data;
  return cx_cmp_cstr(&xs, &ys);
}

static bool ok_imp(struct cx_box *v) {
  return v->as_str->len;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_str = cx_str_ref(src->as_str);
}

static void clone_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_str = cx_str_new(src->as_str->data);
}

static struct cx_iter *iter_imp(struct cx_box *v) {
  return char_iter_new(v->as_str);
}

static void write_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "'%s'", v->as_str->data);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "'%s'@%d", v->as_str->data, v->as_str->nrefs);
}

static void print_imp(struct cx_box *v, FILE *out) {
  fputs(v->as_str->data, out);
}

static void deinit_imp(struct cx_box *v) {
  cx_str_deref(v->as_str);
}

struct cx_type *cx_init_str_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Str", cx->cmp_type, cx->seq_type);
  t->eqval = eqval_imp;
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->clone = clone_imp;
  t->iter = iter_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->print = print_imp;
  t->deinit = deinit_imp;

  return t;
}
