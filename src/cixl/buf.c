#include <ctype.h>

#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/malloc.h"

struct cx_buf *cx_buf_new(struct cx *cx) {
  struct cx_buf *b = cx_malloc(&cx->buf_alloc);
  b->cx = cx;
  cx_vec_init(&b->data, sizeof(unsigned char));
  b->nrefs = 1;
  return b;
}

struct cx_buf *cx_buf_ref(struct cx_buf *b) {
  b->nrefs++;
  return b;
}

void cx_buf_deref(struct cx_buf *b) {
  cx_test(b->nrefs);
  b->nrefs--;

  if (!b->nrefs) {
    cx_vec_deinit(&b->data);
    cx_free(&b->cx->buf_alloc, b);
  }
}

static void new_imp(struct cx_box *out) {
  out->as_buf = cx_buf_new(out->type->lib->cx);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_buf == y->as_buf;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_buf = cx_buf_ref(src->as_buf);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  struct cx_buf *b = v->as_buf;
  fputs("Buf(", out);
  cx_do_vec(&b->data, unsigned char, c) {
    if (isgraph(*c)) {
      fputc(*c, out);
    } else {
      fprintf(out, "@%d", *c); 
    }
  }

  fputc(')', out);
}

static void deinit_imp(struct cx_box *v) {
  cx_buf_deref(v->as_buf);
}

struct cx_type *cx_init_buf_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Buf", cx->any_type);
  t->new = new_imp;
  t->equid = equid_imp;
  t->copy = copy_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
