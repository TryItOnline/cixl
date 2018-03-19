#include <stdlib.h>

#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/lib.h"
#include "cixl/malloc.h"
#include "cixl/mfile.h"
#include "cixl/posix/memstream.h"
#include "cixl/type.h"

static FILE *mopen(char **data, size_t *size) {
  return open_memstream(data, size);
}

struct cx_mfile *cx_mfile_open(struct cx_mfile *f) {
  f->data = NULL;
  f->size = 0;
  f->stream = mopen(&f->data, &f->size);
  return f;
}

struct cx_mfile *cx_mfile_close(struct cx_mfile *f) {
  cx_test(f->stream);
  fclose(f->stream);
  f->stream = NULL;
  return f;
}

static void new_imp(struct cx_box *out) {
  struct cx *cx = out->type->lib->cx;
  struct cx_mfile_ref *mfr = cx_malloc(&cx->mfile_alloc);
  mfr->data = NULL;
  mfr->size = 0;
  FILE *mf = mopen(&mfr->data, &mfr->size);
  cx_file_init(&mfr->file, cx, -1, "w+", mf);
  out->as_file = &mfr->file;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "MFile(%p)r%d", v->as_file, v->as_file->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  struct cx *cx = v->type->lib->cx;
  struct cx_file *f = v->as_file;

  cx_test(f->nrefs);
  f->nrefs--;
  
  if (!f->nrefs) {
    cx_file_close(f);
    struct cx_mfile_ref *mf = cx_baseof(f, struct cx_mfile_ref, file);
    if (mf->data) { free(mf->data); }
    cx_free(&cx->mfile_alloc, mf);
  }
}

struct cx_type *cx_init_mfile_type(struct cx_lib *lib) {
  struct cx_type *t = cx_init_file_type(lib, "MFile", lib->cx->rwfile_type);
  t->new = new_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
