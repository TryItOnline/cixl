#ifndef CX_BUF_H
#define CX_BUF_H

#include "cixl/vec.h"

struct cx;
struct cx_lib;
struct cx_type;

struct cx_buf {
  struct cx *cx;
  struct cx_vec data;
  unsigned int nrefs;
};

struct cx_buf *cx_buf_new(struct cx *cx);
struct cx_buf *cx_buf_ref(struct cx_buf *b);
void cx_buf_deref(struct cx_buf *b);

struct cx_type *cx_init_buf_type(struct cx_lib *lib);

#endif
