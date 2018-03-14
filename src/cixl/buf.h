#ifndef CX_BUF_H
#define CX_BUF_H

#include "cixl/vec.h"

struct cx;
struct cx_lib;
struct cx_type;

struct cx_buf {
  struct cx *cx;
  struct cx_vec data;
  size_t pos;
  unsigned int nrefs;
};

struct cx_buf *cx_buf_new(struct cx *cx);
struct cx_buf *cx_buf_ref(struct cx_buf *b);
void cx_buf_deref(struct cx_buf *b);

void cx_buf_push_char(struct cx_buf *b, unsigned char c);
void cx_buf_push_str(struct cx_buf *b, const char *s);

void cx_buf_clear(struct cx_buf *b);
unsigned char *cx_buf_ptr(struct cx_buf *b);
size_t cx_buf_len(struct cx_buf *b);

struct cx_type *cx_init_buf_type(struct cx_lib *lib);

#endif
