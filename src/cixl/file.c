#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/iter.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/file.h"

struct char_iter {
  struct cx_iter iter;
  struct cx_file *in;
};

static bool char_next(struct cx_iter *iter,
		      struct cx_box *out,
		      struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct char_iter *it = cx_baseof(iter, struct char_iter, iter);
  FILE *fptr = cx_file_ptr(it->in);
  int c = fgetc(fptr);

  if (c == EOF) {
    if (feof(fptr)) {
      iter->done = true;
      return false;
    } else if (errno == EAGAIN) {
      cx_box_init(out, cx->nil_type);
    } else {
      cx_error(cx, cx->row, cx->col, "Failed reading char: %d", errno);
      return false;
    }
  } else {
    cx_box_init(out, cx->char_type)->as_char = c;
  }
  
  return true;
}

static void *char_deinit(struct cx_iter *iter) {
  struct char_iter *it = cx_baseof(iter, struct char_iter, iter);
  cx_file_deref(it->in);
  return it;
}

static cx_iter_type(char_iter, {
    type.next = char_next;
    type.deinit = char_deinit;
  });

static struct cx_iter *char_iter_new(struct cx_file *in) {
  struct char_iter *it = malloc(sizeof(struct char_iter));
  cx_iter_init(&it->iter, char_iter());
  it->in = cx_file_ref(in);
  return &it->iter;
}

struct cx_file *cx_file_new(struct cx *cx, int fd, const char *mode, FILE *ptr) {
  return cx_file_init(cx_malloc(&cx->file_alloc), cx, fd, mode, ptr);
}

struct cx_file *cx_file_ref(struct cx_file *file) {
  file->nrefs++;
  return file;
}

void cx_file_deref(struct cx_file *f) {
  cx_test(f->nrefs);
  f->nrefs--;
  if (!f->nrefs) { cx_free(&f->cx->file_alloc, cx_file_deinit(f)); }
}

struct cx_file *cx_file_init(struct cx_file *f,
			     struct cx *cx,
			     int fd,
			     const char *mode,
			     FILE *ptr) {
  f->cx = cx;
  f->fd = fd;
  f->mode = mode;
  f->_ptr = ptr;
  f->nrefs = 1;
  return f;
}

struct cx_file *cx_file_deinit(struct cx_file *f) {
  cx_file_close(f);
  return f;
}

FILE *cx_file_ptr(struct cx_file *file) {
  if (!file->_ptr) { file->_ptr = fdopen(file->fd, file->mode); }
  return file->_ptr;
}

bool cx_file_unblock(struct cx_file *file) {
  if (fcntl(file->fd, F_SETFL, fcntl(file->fd, F_GETFL, 0) | O_NONBLOCK) == -1) {
    struct cx *cx = file->cx;
    cx_error(cx, cx->row, cx->col, "Failed unblocking file: %d", errno);
    return false;
  }

  return true;
}

bool cx_file_close(struct cx_file *file) {
  if (file->_ptr == stdin || file->_ptr == stdout) { return true; }
  
  if (file->_ptr) {
    fclose(file->_ptr);
    file->_ptr = NULL;
  } else if (file->fd != -1) {
    close(file->fd);
  } else {
    return false;
  }

  file->fd = -1;
  return true;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_file == y->as_file;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  return cx_cmp_ptr(&x->as_file, &y->as_file);
}

static bool ok_imp(struct cx_box *v) {
  return v->as_file->fd != -1;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_file = cx_file_ref(src->as_file);
}

struct cx_iter *cx_file_iter(struct cx_box *v) {
  return char_iter_new(v->as_file);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "%s(%p)r%d", v->type->id, v->as_file, v->as_file->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_file_deref(v->as_file);
}

struct cx_type *_cx_init_file_type(struct cx_lib *lib, const char *name, ...) {
  va_list parents;
  va_start(parents, name);				
  struct cx_type *t = cx_vadd_type(lib, name, parents);
  va_end(parents);

  cx_derive(t, lib->cx->cmp_type);
  
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
