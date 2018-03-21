#ifndef CX_BUF_H
#define CX_BUF_H

#include "cixl/file.h"

struct cx;
struct cx_lib;
struct cx_type;

struct cx_buf {
  struct cx_file file;
  char *data;
  size_t len, pos;  
};

struct cx_buf *cx_buf_new(struct cx *cx);
size_t cx_buf_len(struct cx_buf *b);
void cx_buf_clear(struct cx_buf *b);

struct cx_type *cx_init_buf_type(struct cx_lib *lib);

#endif
