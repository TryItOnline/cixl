#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/poll.h"
#include "cixl/queue.h"
#include "cixl/scope.h"

struct cx_queue *cx_queue_new(struct cx *cx) {
  struct cx_queue *q = malloc(sizeof(struct cx_queue));
  q->cx = cx;
 
  if (pipe(q->fds) == -1) {
    cx_error(cx, cx->row, cx->col, "Failed creating pipe: %d", errno);
    return NULL;
  }

  cx_vec_init(&q->buf, sizeof(struct cx_box));
  q->read_pos = q->write_pos = 0;
  
  cx_vec_init(&q->procs, sizeof(struct cx_box));
  q->proc_pos = 0;
  
  cx_set_init(&q->polls, sizeof(struct cx_poll *), cx_cmp_ptr);
  q->nrefs = 1;
  return q;
}

struct cx_queue *cx_queue_ref(struct cx_queue *q) {
  q->nrefs++;
  return q;
}

void cx_queue_deref(struct cx_queue *q) {
  cx_test(q->nrefs);
  q->nrefs--;

  if (!q->nrefs) {
    cx_do_set(&q->polls, struct cx_poll *, p) {
      cx_poll_delete(*p, q->fds[0]);
      cx_poll_delete(*p, q->fds[1]);
      cx_poll_deref(*p);
    }
    
    cx_set_deinit(&q->polls);    

    cx_do_vec(&q->buf, struct cx_box, v) {
      if (q->read_pos) {
	q->read_pos--;
      } else {
	cx_box_deinit(v);
      }
    }
    
    cx_vec_deinit(&q->buf);

    cx_do_vec(&q->procs, struct cx_box, p) { cx_box_deinit(p); }
    cx_vec_deinit(&q->procs);
    
    close(q->fds[0]);
    close(q->fds[1]);
    free(q);
  }
}

static bool on_read(void *_q) {
  struct cx_queue *q = _q;
  if (!q->procs.count) { return true; }
  
  const int n = q->buf.count-q->read_pos;
  unsigned char tmp[n];
  int rn = read(q->fds[0], tmp, n);

  if (rn == -1) {
    if (errno == EAGAIN) { return true; }
    cx_error(q->cx, q->cx->row, q->cx->col, "Failed reading: %d", errno);
    return false;
  }

  while (rn--) {
    struct cx_box
      *v = cx_vec_get(&q->buf, q->read_pos++),
      *qp = cx_vec_get(&q->procs, q->proc_pos++);
    
    if (q->proc_pos == q->procs.count) { q->proc_pos = 0; }

    struct cx_scope *s = cx_scope(q->cx, 0);
    *cx_push(s) = *v;
    if (!cx_call(qp, s)) { return false; } 
  }

  if (q->read_pos == q->buf.count) {
    cx_vec_clear(&q->buf);
    q->read_pos = q->write_pos = 0;
  }
  
  return true;
}

static bool on_write(void *_q) {
  struct cx_queue *q = _q;
  const int n = q->buf.count-q->write_pos;
  unsigned char tmp[n];
  memset(tmp, 0, sizeof(tmp));
  int wn = write(q->fds[1], tmp, n);

  if (wn == -1) {
    if (errno == EAGAIN) { return true; }
    cx_error(q->cx, q->cx->row, q->cx->col, "Failed writing: %d", errno);
    return false;
  }

  q->write_pos += wn;
  
  if (q->write_pos == q->buf.count) {
    cx_do_set(&q->polls, struct cx_poll *, p) { cx_poll_delete(*p, q->fds[1]); }
  }

  return true;
}

void cx_queue_push(struct cx_queue *q, struct cx_box *val) {
  if (q->write_pos == q->buf.count) {
    cx_do_set(&q->polls, struct cx_poll *, p) {
      struct cx_poll_file *pf = cx_poll_write(*p, q->fds[1]);
      pf->write_fn = on_write;
      pf->write_data = q;
    }
  }

  cx_copy(cx_vec_push(&q->buf), val);
}

void cx_queue_pop(struct cx_queue *q, struct cx_box *proc) {
  cx_copy(cx_vec_push(&q->procs), proc);
}

void cx_queue_poll(struct cx_queue *q, struct cx_poll *poll) {
  struct cx_poll **pok = cx_set_insert(&q->polls, &poll);

  if (pok) {
    *pok = cx_poll_ref(poll);
    struct cx_poll_file *pf = cx_poll_read(poll, q->fds[0]);
    pf->read_fn = on_read;
    pf->read_data = q;
    
    if (q->buf.count) {
      pf = cx_poll_write(poll, q->fds[1]);
      pf->write_fn = on_write;
      pf->write_data = q;
    }
  }
}

size_t cx_queue_len(struct cx_queue *q) {
  return q->buf.count-q->read_pos;
}

static void new_imp(struct cx_box *out) {
  out->as_queue = cx_queue_new(out->type->lib->cx);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_queue == y->as_queue;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_queue = cx_queue_ref(src->as_queue);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  struct cx_queue *q = v->as_queue;
  fprintf(out, "Queue(%p)r%d", q, q->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_queue_deref(v->as_queue);
}

struct cx_type *cx_init_queue_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Queue", cx->any_type);
  t->new = new_imp;
  t->equid = equid_imp;
  t->copy = copy_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
