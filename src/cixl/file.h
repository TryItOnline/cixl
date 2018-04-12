#ifndef CX_FILE_H
#define CX_FILE_H

#include <stdbool.h>
#include <stdio.h>

#define cx_init_file_type(cx, name, ...)		\
  _cx_init_file_type(cx, name, ##__VA_ARGS__, NULL)	\

struct cx;
struct cx_box;
struct cx_lib;
struct cx_type;

struct cx_file {
  struct cx *cx;
  int fd;
  const char *mode;
  FILE *_ptr;
  unsigned int nrefs;
};

struct cx_file *cx_file_new(struct cx *cx, int fd, const char *mode, FILE *ptr);
struct cx_file *cx_file_ref(struct cx_file *file);
bool cx_file_deref(struct cx_file *file);

struct cx_file *cx_file_init(struct cx_file *f,
			     struct cx *cx,
			     int fd,
			     const char *mode,
			     FILE *ptr);

struct cx_file *cx_file_deinit(struct cx_file *f);

FILE *cx_file_ptr(struct cx_file *file);
void cx_file_iter(struct cx_box *in, struct cx_box *out);
bool cx_file_unblock(struct cx_file *file);
bool cx_file_close(struct cx_file *file);

struct cx_type *_cx_init_file_type(struct cx_lib *lib, const char *name, ...);

#endif
