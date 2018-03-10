#ifndef CX_MFILE_H
#define CX_MFILE_H

#include <stdio.h>

struct cx_mfile {
  char *data;
  size_t size;
  FILE *stream;
};

struct cx_mfile *cx_mfile_open(struct cx_mfile *f);
struct cx_mfile *cx_mfile_close(struct cx_mfile *f);

#endif
