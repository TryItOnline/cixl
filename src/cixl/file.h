#ifndef CX_FILE_H
#define CX_FILE_H

#include <stdio.h>

#define cx_init_file_type(cx, name, ...)		\
  _cx_init_file_type(cx, name, ##__VA_ARGS__, NULL)	\

struct cx;
struct cx_type;

struct cx_file {
  FILE *ptr;
  unsigned int nrefs;
};

struct cx_file *cx_file_new(FILE *ptr);
struct cx_file *cx_file_ref(struct cx_file *file);
void cx_file_deref(struct cx_file *file);

struct cx_iter *cx_file_iter(struct cx_box *v);

struct cx_type *_cx_init_file_type(struct cx_lib *lib, const char *name, ...);

#endif
