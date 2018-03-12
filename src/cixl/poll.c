#include <poll.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/file.h"
#include "cixl/poll.h"

static struct cx_poll_file *file_init(struct cx_poll_file *pf, int fd) {
  pf->fd = fd;
  pf->read_data = pf->write_data = NULL;
  pf->read_fn = pf->write_fn = NULL;
  return pf;
}

static void file_deinit(struct cx_poll_file *f, struct pollfd *pfd) {
  if (pfd->events & POLLIN && !f->read_fn) { cx_box_deinit(&f->read_value); }
  if (pfd->events & POLLOUT && !f->write_fn) { cx_box_deinit(&f->write_value); }
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

struct cx_poll_file *cx_poll_read(struct cx_poll *p, int fd) {
  void *found = NULL;
  size_t i = cx_set_find(&p->files, &fd, 0, &found);
  
  struct cx_poll_file *pf = NULL;
  struct pollfd *pfd = NULL;
  
  if (found) {
    pf = found;
    pfd = cx_vec_get(&p->fds.members, i);
    
    if (pfd->events & POLLIN) {
      if (pf->read_fn) {
	pf->read_fn = NULL;
	pf->read_data = NULL;
      } else {
	cx_box_deinit(&pf->read_value);
      }
    }
  } else {
    pf = file_init(cx_vec_insert(&p->files.members, i), fd);
    pfd = cx_vec_insert(&p->fds.members, i);
    pfd->fd = fd;
    pfd->events = 0;
  }

  pfd->events |= POLLIN;
  return pf;
}

struct cx_poll_file *cx_poll_write(struct cx_poll *p, int fd) {
  void *found = NULL;
  size_t i = cx_set_find(&p->files, &fd, 0, &found);
  
  struct cx_poll_file *pf = NULL;
  struct pollfd *pfd = NULL;
  
  if (found) {
    pf = found;
    pfd = cx_vec_get(&p->fds.members, i);
    
    if (pfd->events & POLLOUT) {
      if (pf->write_fn) {
	pf->write_fn = NULL;
	pf->write_data = NULL;
      } else {
	cx_box_deinit(&pf->write_value);
      }
    }
  } else {
    pf = file_init(cx_vec_insert(&p->files.members, i), fd);
    pfd = cx_vec_insert(&p->fds.members, i);
    pfd->fd = fd;
    pfd->events = 0;
  }

  pfd->events |= POLLOUT;
  return pf;
}

bool cx_poll_delete(struct cx_poll *p, int fd) {
  void *found = NULL;
  size_t i = cx_set_find(&p->files, &fd, 0, &found);
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
      if (fd->revents & POLLIN) {
	if (f->read_fn) {
	  if (!f->read_fn(f->read_data)) { return -1; }
	} else if (!cx_call(&f->read_value, s)) {
	  return -1;
	}
	
	rem--;
      }

      if (fd->revents & POLLOUT) {
	if (f->write_fn) {
	  if (!f->write_fn(f->write_data)) { return -1; }
	} else if (!cx_call(&f->write_value, s)) {
	  return -1;
	}

	rem--;
      }      
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
