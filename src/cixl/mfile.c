#include <stdlib.h>

#include "cixl/mfile.h"
#include "cixl/error.h"
#include "cixl/posix/memstream.h"

struct cx_mfile *cx_mfile_open(struct cx_mfile *f) {
  f->data = NULL;
  f->size = 0;
  f->stream = open_memstream(&f->data, &f->size);
  return f;
}

struct cx_mfile *cx_mfile_close(struct cx_mfile *f) {
  cx_test(f->stream);
  fclose(f->stream);
  f->stream = NULL;
  return f;
}
