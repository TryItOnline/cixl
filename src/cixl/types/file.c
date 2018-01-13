#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/file.h"

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

void cx_file_unref(struct cx_file *file) {
  cx_test(file->nrefs > 0);
  file->nrefs--;
  
  if (!file->nrefs) {
    if (file->ptr != stdin && file->ptr != stdout) { fclose(file->ptr); }
    free(file);
  }
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_file == y->as_file;
}

static bool ok_imp(struct cx_box *v) {
  return feof(v->as_file->ptr);
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_file = cx_file_ref(src->as_file);
}

static void print_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "%s(%p)@%d", v->type->id, v->as_file, v->as_file->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_file_unref(v->as_file);
}

struct cx_type *cx_init_file_type(struct cx *cx,
				  const char *name,
				  struct cx_type *parent) {
  struct cx_type *t = cx_add_type(cx, name, parent);
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->print = print_imp;
  t->deinit = deinit_imp;
  return t;
}
