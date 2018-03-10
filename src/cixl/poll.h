#ifndef CX_POLL_H
#define CX_POLL_H

#include "cixl/set.h"

struct cx;
struct cx_box;
struct cx_file;

struct cx_poll {
  struct cx_set files, fds;
};

struct cx_poll *cx_poll_init(struct cx_poll *poll);
struct cx_poll *cx_poll_deinit(struct cx_poll *poll);

bool cx_poll_read(struct cx_poll *poll, struct cx_file *file, struct cx_box *act);
bool cx_poll_clear(struct cx_poll *poll, struct cx_file *file);
int cx_poll_wait(struct cx_poll *poll, int msecs, struct cx_scope *scope);

#endif
