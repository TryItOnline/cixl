#include <stdlib.h>

#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/lib.h"
#include "cixl/malloc.h"
#include "cixl/mfile.h"
#include "cixl/posix/memstream.h"
#include "cixl/type.h"

FILE *cx_mopen(char **data, size_t *len) {
  return open_memstream(data, len);
}

struct cx_mfile *cx_mfile_open(struct cx_mfile *f) {
  f->data = NULL;
  f->size = 0;
  f->stream = cx_mopen(&f->data, &f->size);
  return f;
}

struct cx_mfile *cx_mfile_close(struct cx_mfile *f) {
  cx_test(f->stream);
  fclose(f->stream);
  f->stream = NULL;
  return f;
}
