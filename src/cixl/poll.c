#include <poll.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/file.h"
#include "cixl/poll.h"

struct cx_poll_file {
  int fd;
  struct cx_file *file;
  struct cx_box on_read;
};

static struct cx_poll_file *file_init(struct cx_poll_file *pf, struct cx_file *f) {
  pf->fd = f->fd;
  pf->file = cx_file_ref(f);
  return pf;
}

static void file_deinit(struct cx_poll_file *f, struct pollfd *pfd) {
  if (pfd->events & POLLIN) { cx_box_deinit(&f->on_read); }
  cx_file_deref(f->file);
}

struct cx_poll *cx_poll_init(struct cx_poll *p) {
  cx_set_init(&p->files, sizeof(struct cx_poll_file), cx_cmp_cint);
  p->files.key = offsetof(struct cx_poll_file, fd);

  cx_set_init(&p->fds, sizeof(struct pollfd), cx_cmp_cint);
  p->fds.key_offs = offsetof(struct pollfd, fd);

  return p;
}

struct cx_poll *cx_poll_deinit(struct cx_poll *p) {
  cx_do_set(&p->files, struct cx_poll_file, f) {
    file_deinit(f, cx_test(cx_set_get(&p->fds, &f->file->fd)));
  }
  
  cx_set_deinit(&p->files);
  cx_set_deinit(&p->fds);
  return p;
}

bool cx_poll_read(struct cx_poll *p, struct cx_file *f, struct cx_box *a) {
  struct cx_poll_file *ok = cx_set_get(&p->files, &f->fd);
  struct pollfd *pfd = NULL;
  
  if (ok) {
    pfd = cx_test(cx_set_get(&p->fds, &f->fd));
    if (pfd->events & POLLIN) { return false; }
  } else {
    ok = file_init(cx_set_insert(&p->files, &f->fd), f);
    pfd = cx_test(cx_set_insert(&p->fds, &f->fd));
    pfd->fd = f->fd;
    pfd->events = 0;
  }

  cx_copy(&ok->on_read, a);
  pfd->events |= POLLIN;
  return true;
}

bool cx_poll_clear(struct cx_poll *p, struct cx_file *f) {
  struct cx_poll_file *ok = cx_set_get(&p->files, &f->fd);
  if (!ok) { return false; }
  file_deinit(ok, cx_set_get(&p->fds, &f->fd));
  cx_test(cx_set_delete(&p->files, &f->fd));
  cx_test(cx_set_delete(&p->fds, &f->fd));
  return true;
}

int cx_poll_wait(struct cx_poll *p, int msecs, struct cx_scope *scope) {
  struct cx_vec *fds = &p->fds.members; 
  int
    num = poll((struct pollfd *)fds->items, fds->count, msecs),
    rem = num;
  
  struct cx_poll_file *f = cx_vec_start(&p->files.members);
  struct pollfd *fd = cx_vec_start(&p->fds.members);
  
  for (; f != cx_vec_end(&p->files.members) && rem;) {
    int prev_fd = f->fd;
    
    if (fd->revents) {
      if ((fd->revents & POLLIN) && !cx_call(&f->on_read, scope)) { return -1; }
      rem--;
    }

    if (f->fd == prev_fd) {
      f++;
      fd++;
    }
  }

  return num;
}
