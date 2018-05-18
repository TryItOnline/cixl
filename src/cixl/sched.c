#include <stdlib.h>

#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/iter.h"
#include "cixl/lib.h"
#include "cixl/sched.h"
#include "cixl/scope.h"
#include "cixl/task.h"
#include "cixl/type.h"

struct cx_sched *cx_sched_new() {
  struct cx_sched *s = malloc(sizeof(struct cx_sched));
  cx_ls_init(&s->tasks);
  s->ntasks = 0;
  s->nrefs = 1;
  return s;
}

struct cx_sched *cx_sched_ref(struct cx_sched *s) {
  s->nrefs++;
  return s;
}

void cx_sched_deref(struct cx_sched *s) {
  cx_test(s->nrefs);
  s->nrefs--;
  
  if (!s->nrefs) {
    cx_do_ls(&s->tasks, tq) {
      free(cx_task_deinit(cx_baseof(tq, struct cx_task, queue)));
    }
    
    free(s);
  }
}

void cx_sched_task(struct cx_sched *s, struct cx_box *action) {
  struct cx_task *t = cx_task_init(malloc(sizeof(struct cx_task)), s, action);
  cx_ls_prepend(&s->tasks, &t->queue);
  s->ntasks++;
}

bool cx_sched_next(struct cx_sched *s, struct cx_scope *scope) {
  if (s->tasks.next == &s->tasks) { return false; }
  struct cx_task *t = cx_baseof(s->tasks.next, struct cx_task, queue);
  cx_ls_delete(&t->queue);    
  if (!cx_task_run(t, scope)) { return false; }
  
  if (t->state == CX_TASK_DONE) {
    free(cx_task_deinit(t));
    s->ntasks--;
  } else {
    cx_ls_prepend(&s->tasks, &t->queue);
  }

  return true;
}

bool cx_sched_run(struct cx_sched *s, struct cx_scope *scope) {
  while (cx_sched_next(s, scope));
  return true;
}

struct task_iter {
  struct cx_iter iter;
  struct cx_sched *sched;
};

static bool task_next(struct cx_iter *iter,
		      struct cx_box *out,
		      struct cx_scope *scope) {
  struct task_iter *it = cx_baseof(iter, struct task_iter, iter);
  struct cx_sched *s = it->sched;
  
  if (s->tasks.next == &s->tasks) {
    iter->done = true;
    return false;
  }

  if (!cx_sched_next(s, scope)) { return false; }
  cx_box_init(out, scope->cx->int_type)->as_int = s->ntasks;
  return true;
}

static void *task_deinit(struct cx_iter *iter) {
  struct task_iter *it = cx_baseof(iter, struct task_iter, iter);
  cx_sched_deref(it->sched);
  return it;
}

static cx_iter_type(task_iter, {
    type.next = task_next;
    type.deinit = task_deinit;
  });

static struct cx_iter *task_iter_new(struct cx_sched *sched) {
  struct task_iter *it = malloc(sizeof(struct task_iter));
  cx_iter_init(&it->iter, task_iter());
  it->sched = cx_sched_ref(sched);
  return &it->iter;
}

static void new_imp(struct cx_box *out) {
  out->as_sched = cx_sched_new();
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_sched == y->as_sched;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_sched = cx_sched_ref(src->as_sched);
}

static void iter_imp(struct cx_box *in, struct cx_box *out) {
  struct cx *cx = in->type->lib->cx;
  
  cx_box_init(out, cx_type_get(cx->iter_type, cx->int_type))->as_iter =
    task_iter_new(in->as_sched);
}

static bool sink_imp(struct cx_box *dst, struct cx_box *v) {
  cx_sched_task(dst->as_sched, v);
  return true;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "Sched(%p)", v->as_sched);
}

static void deinit_imp(struct cx_box *v) {
  cx_sched_deref(v->as_sched);
}

struct cx_type *cx_init_sched_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  
  struct cx_type *t = cx_add_type(lib, "Sched",
				  cx->sink_type,
				  cx_type_get(cx->seq_type, cx->int_type));

  t->new = new_imp;
  t->equid = equid_imp;
  t->copy = copy_imp;
  t->iter = iter_imp;
  t->sink = sink_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
