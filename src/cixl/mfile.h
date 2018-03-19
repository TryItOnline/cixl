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

struct cx_mfile *cx_mfile_open(struct cx_mfile *f);
struct cx_mfile *cx_mfile_close(struct cx_mfile *f);

struct cx_mfile_ref {
  struct cx_file file;
  char *data;
  size_t size;  
};

struct cx_type *cx_init_mfile_type(struct cx_lib *lib);

#endif
