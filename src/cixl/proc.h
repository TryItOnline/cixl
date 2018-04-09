#ifndef CX_PROC_H
#define CX_PROC_H

#include <stdbool.h>

struct cx;
struct cx_file;

struct cx_proc {
  struct cx *cx;
  pid_t pid;
  int in_fd, out_fd, error_fd;
  struct cx_file *in, *out, *error;
  int nrefs;
};	       

struct cx_proc *cx_proc_new(struct cx *cx);
struct cx_proc *cx_proc_init(struct cx_proc *p, struct cx *cx);
struct cx_proc *cx_proc_deinit(struct cx_proc *p);

struct cx_proc *cx_proc_ref(struct cx_proc *p);
void cx_proc_deref(struct cx_proc *p);

int cx_proc_fork(struct cx_proc *p,
		 struct cx_file *in, struct cx_file *out, struct cx_file *error);

bool cx_proc_wait(struct cx_proc *p, int ms, struct cx_box *status);
bool cx_proc_kill(struct cx_proc *p, int ms, struct cx_box *status);

struct cx_type *cx_init_proc_type(struct cx_lib *lib);

#endif
