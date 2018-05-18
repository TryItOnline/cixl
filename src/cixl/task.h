#ifndef CX_TASK_H
#define CX_TASK_H

#include "cixl/box.h"
#include "cixl/compat/ucontext.h"
#include "cixl/ls.h"

#define CX_TASK_STACK_SIZE 32000

struct cx_sched;
struct cx_scope;

enum cx_task_state {CX_TASK_NEW, CX_TASK_RUN, CX_TASK_DONE};

struct cx_task {
  struct cx_sched *sched;
  struct cx_box action;
  ucontext_t context;
  enum cx_task_state state;
  struct cx_task *prev_task;
  ssize_t prev_pc, pc;
  struct cx_bin *prev_bin, *bin;
  ssize_t prev_nlibs, prev_nscopes, prev_ncalls;
  struct cx_vec libs, scopes, calls;
  char stack[CX_TASK_STACK_SIZE];
  struct cx_ls queue;
};

struct cx_task *cx_task_init(struct cx_task *t,
			     struct cx_sched *sched,
			     struct cx_box *action);

struct cx_task *cx_task_deinit(struct cx_task *t);
bool cx_task_resched(struct cx_task *t, struct cx_scope *scope);
bool cx_task_run(struct cx_task *t, struct cx_scope *scope);

#endif
