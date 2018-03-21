#ifndef CX_MFILE_H
#define CX_MFILE_H

#include "cixl/file.h"
#include <stdio.h>

struct cx_lib;
struct cx_type;

struct cx_mfile {
  char *data;
  size_t size;
  FILE *stream;
};

FILE *cx_mopen(char **data, size_t *len);
struct cx_mfile *cx_mfile_open(struct cx_mfile *f);
struct cx_mfile *cx_mfile_close(struct cx_mfile *f);

#endif
