#ifndef CX_POLL_H
#define CX_POLL_H

#include "cixl/box.h"
#include "cixl/set.h"

struct cx;
struct cx_file;
struct cx_type;

struct cx_poll_file {
  int fd;

  bool (*read_fn)(void *);
  bool (*write_fn)(void *);
  void *read_data, *write_data;

  struct cx_box read_value, write_value;
};

struct cx_poll {
  struct cx_set files, fds;
  unsigned int nrefs;
};

struct cx_poll *cx_poll_new();
struct cx_poll *cx_poll_ref(struct cx_poll *p);
void cx_poll_deref(struct cx_poll *p);

struct cx_poll_file *cx_poll_read(struct cx_poll *p, int fd);
struct cx_poll_file *cx_poll_write(struct cx_poll *p, int fd);

bool cx_poll_delete(struct cx_poll *p, int fd);
int cx_poll_wait(struct cx_poll *p, int ms, struct cx_scope *s);

struct cx_type *cx_init_poll_type(struct cx_lib *lib);

#endif
