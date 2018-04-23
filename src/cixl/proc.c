#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/file.h"
#include "cixl/proc.h"
#include "cixl/timer.h"

struct cx_proc *cx_proc_new(struct cx *cx) {
  return cx_proc_init(malloc(sizeof(struct cx_proc)), cx);
}

struct cx_proc *cx_proc_init(struct cx_proc *p, struct cx *cx) {
  p->cx = cx;
  p->pid = p->in_fd = p->out_fd = p->error_fd = -1;
  p->in = p->out = p->error = NULL;
  p->nrefs = 1;
  return p;
}

struct cx_proc *cx_proc_deinit(struct cx_proc *p) {
  if (p->in) {
    cx_file_deref(p->in);
  } else if (p->in_fd != -1) {
    close(p->in_fd);
  }
  
  if (p->out) {
    cx_file_deref(p->out);
  } else if (p->out_fd != -1) {
    close(p->out_fd);
  }

  if (p->error) {
    cx_file_deref(p->error);
  } else if (p->error_fd != -1) {
    close(p->error_fd);
  }
  
  return p;
}

struct cx_proc *cx_proc_ref(struct cx_proc *p) {
  p->nrefs++;
  return p;
}

void cx_proc_deref(struct cx_proc *p) {
  cx_test(p->nrefs);
  p->nrefs--;
  if (!p->nrefs) { free(cx_proc_deinit(p)); }
}

int cx_proc_fork(struct cx_proc *p,
		 struct cx_file *in, struct cx_file *out, struct cx_file *error) {
  int in_fds[2], out_fds[2], error_fds[2];

  if (!in) {
    if (pipe(in_fds) == -1) {
      cx_error(p->cx, p->cx->row, p->cx->col, "Failed creating pipe: %d", errno);
      return -1;
    }
    
    p->in_fd = in_fds[1];
  }

  if (!out) {
    if (pipe(out_fds) == -1) {
      cx_error(p->cx, p->cx->row, p->cx->col, "Failed creating pipe: %d", errno);
      close(in_fds[0]);
      return -1;
    }

    p->out_fd = out_fds[0];
  }

  if (!error) {
    if (pipe(error_fds) == -1) {
      cx_error(p->cx, p->cx->row, p->cx->col, "Failed creating pipe: %d", errno);
      close(in_fds[0]);
      close(out_fds[1]);
      return -1;
    }

    p->error_fd = error_fds[0];
  }
  
  p->pid = fork();
        
  if (p->pid) {
    if (p->pid == -1) {
      cx_error(p->cx, p->cx->row, p->cx->col, "Failed forking: %d", errno);
    }

    if (!in) { close(in_fds[0]); }
    if (!out) { close(out_fds[1]); }
    if (!error) { close(error_fds[1]); }

    return p->pid;
  }

  if (!in || in->fd != STDIN_FILENO) {
    if (dup2(in ? in->fd : in_fds[0], STDIN_FILENO) == -1) {
      cx_error(p->cx, p->cx->row, p->cx->col, "Failed setting stdin: %d", errno);
      return -1;
    }

    if (in) {
      cx_file_close(in);
    } else {
      close(in_fds[0]);
      close(in_fds[1]);
    }
  }

  if (!out || out->fd != STDOUT_FILENO) {
    if (dup2(out ? out->fd : out_fds[1], STDOUT_FILENO) == -1) {
      cx_error(p->cx, p->cx->row, p->cx->col, "Failed setting stdout: %d", errno);
      return -1;
    }

    if (out) {
      cx_file_close(out);
    } else {
      close(out_fds[0]);
      close(out_fds[1]);
    }
  }

  if (!error || error->fd != STDOUT_FILENO) {
    if (dup2(error ? error->fd : error_fds[1], STDERR_FILENO) == -1) {
      cx_error(p->cx, p->cx->row, p->cx->col, "Failed setting stderr: %d", errno);
      return -1;
    }

    if (error) {
      cx_file_close(error);
    } else {
      close(error_fds[0]);
      close(error_fds[1]);
    }
  }

  return 0;
}

bool cx_proc_wait(struct cx_proc *p, int ms, struct cx_box *status) {
  if (p->pid == -1) {
    cx_error(p->cx, p->cx->row, p->cx->col, "Invalid pid: -1", errno);
    return false;
  }

  cx_timer_t t; 
  if (ms != -1) { cx_timer_reset(&t); }
  
  while (ms == -1 || (cx_timer_ns(&t) / 1000) < ms) {
    int s = -1;
    pid_t ok = waitpid(p->pid, &s, WNOHANG);    
    if (ok == -1 && errno != EINTR) { break; }

    if (ok > 0) {
      if (WIFEXITED(s)) {
	cx_box_init(status, p->cx->int_type)->as_int = WEXITSTATUS(s);
      } else {
	cx_box_init(status, p->cx->nil_type);
      }
      
      p->pid = -1;
      return true;
    }
  }

  cx_error(p->cx, p->cx->row, p->cx->col, "Failed waiting: %d", errno);
  return false;
}

bool cx_proc_kill(struct cx_proc *p, int ms, struct cx_box *status) {
  if (p->pid == -1) {
    cx_error(p->cx, p->cx->row, p->cx->col, "Invalid pid: -1", errno);
    return false;
  }
  
  if (kill(p->pid, SIGTERM) == -1) {
    cx_error(p->cx, p->cx->row, p->cx->col, "Failed killing: %d", errno);
    return false;
  }

  cx_box_init(status, p->cx->nil_type);

  if (ms && !cx_proc_wait(p, ms, status) && kill(p->pid, SIGKILL) == -1) {
    cx_error(p->cx, p->cx->row, p->cx->col, "Failed killing: %d", errno);
    return false;
  }

  p->pid = -1;
  return true;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_proc == y->as_proc;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  return cx_cmp_cint(&x->as_proc->pid, &y->as_proc->pid);
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_proc = cx_proc_ref(src->as_proc);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "Proc(%d)", v->as_proc->pid);
}

static void deinit_imp(struct cx_box *v) {
  cx_proc_deref(v->as_proc);
}

struct cx_type *cx_init_proc_type(struct cx_lib *lib) {
  struct cx_type *t = cx_add_type(lib, "Proc", lib->cx->cmp_type);
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->copy = copy_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
