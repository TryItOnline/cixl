#include <poll.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/file.h"
#include "cixl/poll.h"

struct cx_poll_file {
  int fd;
  struct cx_box on_read;
};

static struct cx_poll_file *file_init(struct cx_poll_file *pf, int fd) {
  pf->fd = fd;
  return pf;
}

static void file_deinit(struct cx_poll_file *f, struct pollfd *pfd) {
  if (pfd->events & POLLIN) { cx_box_deinit(&f->on_read); }
}

struct cx_poll *cx_poll_new() {
  struct cx_poll *p = malloc(sizeof(struct cx_poll));
  cx_set_init(&p->files, sizeof(struct cx_poll_file), cx_cmp_cint);
  p->files.key = offsetof(struct cx_poll_file, fd);
  cx_set_init(&p->fds, sizeof(struct pollfd), cx_cmp_cint);
  p->fds.key_offs = offsetof(struct pollfd, fd);
  p->nrefs = 1;
  return p;
}

struct cx_poll *cx_poll_ref(struct cx_poll *p) {
  p->nrefs++;
  return p;
}

void cx_poll_deref(struct cx_poll *p) {
  cx_test(p->nrefs);
  p->nrefs--;

  if (!p->nrefs) {
    struct cx_poll_file *f = cx_vec_start(&p->files.members);
    struct pollfd *fd = cx_vec_start(&p->fds.members);

    for (; f != cx_vec_end(&p->files.members); f++, fd++) {
      file_deinit(f, cx_test(cx_set_get(&p->fds, &f->fd)));
    }
    
    cx_set_deinit(&p->files);
    cx_set_deinit(&p->fds);
    free(p);
  }
}

bool cx_poll_read(struct cx_poll *p, struct cx_file *f, struct cx_box *a) {
  void *found = NULL;
  size_t i = cx_set_find(&p->files, &f->fd, 0, &found);
  
  struct cx_poll_file *pf = NULL;
  struct pollfd *pfd = NULL;
  
  if (found) {
    pf = found;
    pfd = cx_test(cx_vec_get(&p->fds.members, i));
    if (pfd->events & POLLIN) { return false; }
  } else {
    pf = file_init(cx_vec_insert(&p->files.members, i), f->fd);
    pfd = cx_vec_insert(&p->fds.members, i);
    pfd->fd = f->fd;
    pfd->events = 0;
    pfd->events = 0;
  }

  cx_copy(&pf->on_read, a);
  pfd->events |= POLLIN;
  return true;
}

bool cx_poll_delete(struct cx_poll *p, struct cx_file *f) {
  void *found = NULL;
  size_t i = cx_set_find(&p->files, &f->fd, 0, &found);
  if (!found) { return false; }
  file_deinit(found, cx_vec_get(&p->fds.members, i));
  cx_vec_delete(&p->files.members, i);
  cx_vec_delete(&p->fds.members, i);
  return true;
}

int cx_poll_wait(struct cx_poll *p, int ms, struct cx_scope *s) {
  struct cx_vec *fds = &p->fds.members; 

  int
    num = poll((struct pollfd *)fds->items, fds->count, ms),
    rem = num;
  
  struct cx_poll_file
    *f = cx_vec_start(&p->files.members),
    *fend = cx_vec_end(&p->files.members);
  
  struct pollfd *fd = cx_vec_start(&p->fds.members);
  
  for (; f != fend && rem;) {
    int prev_fd = f->fd;
    
    if (fd->revents) {
      if ((fd->revents & POLLIN) && !cx_call(&f->on_read, s)) { return -1; }
      rem--;
    }

    if (f->fd == prev_fd) {
      f++;
      fd++;
    }
  }

  return num;
}

static void new_imp(struct cx_box *out) {
  out->as_poll = cx_poll_new();
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_poll == y->as_poll;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_poll = cx_poll_ref(src->as_poll);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  struct cx_poll *p = v->as_poll;
  fprintf(out, "Poll(%p)r%d", p, p->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_poll_deref(v->as_poll);
}

struct cx_type *cx_init_poll_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Poll", cx->any_type);
  t->new = new_imp;
  t->equid = equid_imp;
  t->copy = copy_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
