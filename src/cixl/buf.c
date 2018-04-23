#include <ctype.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/malloc.h"
#include "cixl/mfile.h"

struct cx_buf *cx_buf_new(struct cx *cx) {
  struct cx_buf *b = cx_malloc(&cx->buf_alloc);
  b->data = NULL;
  b->len = 0;
  FILE *f = cx_mopen(&b->data, &b->len);
  cx_file_init(&b->file, cx, -1, "w+", f);
  b->pos = 0;
  return b;
}

size_t cx_buf_len(struct cx_buf *b) {
  return ftell(b->file._ptr) - b->pos;
}

void cx_buf_clear(struct cx_buf *b) {
  fseek(b->file._ptr, 0, SEEK_SET);
  b->pos = 0;
}

static void new_imp(struct cx_box *out) {
  out->as_file = &cx_buf_new(out->type->lib->cx)->file;
}

static bool ok_imp(struct cx_box *v) {
  struct cx_buf *b = cx_baseof(v->as_file, struct cx_buf, file);
  return cx_buf_len(b);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "Buf(%p)", v->as_file);
}

static void deinit_imp(struct cx_box *v) {
  struct cx_file *f = v->as_file;
  cx_test(f->nrefs);
  f->nrefs--;

  if (!f->nrefs) {
    cx_file_close(f);
    struct cx_buf *b = cx_baseof(f, struct cx_buf, file);
    if (b->data) { free(b->data); }
    cx_free(&v->type->lib->cx->buf_alloc, b);
  }
}

struct cx_type *cx_init_buf_type(struct cx_lib *lib) {
  struct cx_type *t = cx_init_file_type(lib, "Buf", lib->cx->rwfile_type);
  t->new = new_imp;
  t->ok = ok_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
