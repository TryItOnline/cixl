#ifndef CX_QUEUE_H
#define CX_QUEUE_H

#include "cixl/file.h"
#include "cixl/set.h"

struct cx_lib;
struct cx_poll;
struct cx_type;

struct cx_queue {
  struct cx *cx;
  int fds[2];

  struct cx_vec buf;
  size_t read_pos, write_pos;

  struct cx_vec procs;
  size_t proc_pos;

  struct cx_set polls;
  unsigned int nrefs;
};

struct cx_queue *cx_queue_new(struct cx *cx);
struct cx_queue *cx_queue_ref(struct cx_queue *q);
void cx_queue_deref(struct cx_queue *q);

void cx_queue_push(struct cx_queue *q, struct cx_box *val);
void cx_queue_pop(struct cx_queue *q, struct cx_box *proc);
void cx_queue_poll(struct cx_queue *q, struct cx_poll *poll);
size_t cx_queue_len(struct cx_queue *q);

struct cx_type *cx_init_queue_type(struct cx_lib *lib);

#endif
