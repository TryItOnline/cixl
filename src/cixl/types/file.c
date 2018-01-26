#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/file.h"
#include "cixl/types/iter.h"

struct char_iter {
  struct cx_iter iter;
  struct cx_file *in;
};

static bool char_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  struct char_iter *it = cx_baseof(iter, struct char_iter, iter);
  int c = fgetc(it->in->ptr);
  
  if (c == EOF) {
    iter->done = true;
    return false;
  }
  
  cx_box_init(out, scope->cx->char_type)->as_char = c;
  return true;
}

static void *char_deinit(struct cx_iter *iter) {
  struct char_iter *it = cx_baseof(iter, struct char_iter, iter);
  cx_file_deref(it->in);
  return it;
}

static cx_iter_type(char_iter, {
    type.next = char_next;
    type.deinit = char_deinit;
  });

static struct cx_iter *char_iter_new(struct cx_file *in) {
  struct char_iter *it = malloc(sizeof(struct char_iter));
  cx_iter_init(&it->iter, char_iter());
  it->in = cx_file_ref(in);
  return &it->iter;
}

struct cx_file *cx_file_new(FILE *ptr) {
  struct cx_file *file = malloc(sizeof(struct cx_file));
  file->ptr = ptr;
  file->nrefs = 1;
  return file;
}

struct cx_file *cx_file_ref(struct cx_file *file) {
  file->nrefs++;
  return file;
}

void cx_file_deref(struct cx_file *file) {
  cx_test(file->nrefs);
  file->nrefs--;
  
  if (!file->nrefs) {
    if (file->ptr != stdin && file->ptr != stdout) { fclose(file->ptr); }
    free(file);
  }
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_file == y->as_file;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  return cx_cmp_ptr(&x->as_file, &y->as_file);
}

static bool ok_imp(struct cx_box *v) {
  return feof(v->as_file->ptr);
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_file = cx_file_ref(src->as_file);
}

struct cx_iter *cx_file_iter(struct cx_box *v) {
  return char_iter_new(v->as_file);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "%s(%p)@%d", v->type->id, v->as_file, v->as_file->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_file_deref(v->as_file);
}

struct cx_type *_cx_init_file_type(struct cx *cx, const char *name, ...) {
  va_list parents;
  va_start(parents, name);				
  struct cx_type *t = cx_vadd_type(cx, name, parents);
  va_end(parents);

  cx_derive(t, cx->cmp_type);
  
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
