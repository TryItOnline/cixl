#ifndef CX_POLL_H
#define CX_POLL_H

#include "cixl/set.h"

struct cx;
struct cx_box;
struct cx_file;
struct cx_type;

struct cx_poll {
  struct cx_set files, fds;
  unsigned int nrefs;
};

struct cx_poll *cx_poll_new();
struct cx_poll *cx_poll_ref(struct cx_poll *p);
void cx_poll_deref(struct cx_poll *p);

bool cx_poll_read(struct cx_poll *p, struct cx_file *f, struct cx_box *a);
bool cx_poll_delete(struct cx_poll *p, struct cx_file *f);
int cx_poll_wait(struct cx_poll *p, int ms, struct cx_scope *s);

struct cx_type *cx_init_poll_type(struct cx_lib *lib);

#endif
