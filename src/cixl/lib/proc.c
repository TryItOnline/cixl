#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "cixl/arg.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/file.h"
#include "cixl/lib.h"
#include "cixl/lib/proc.h"
#include "cixl/proc.h"
#include "cixl/scope.h"
#include "cixl/stack.h"
#include "cixl/str.h"

static bool fork_imp(struct cx_call *call) {
  struct cx_box 
    *error = cx_test(cx_call_arg(call, 2)),
    *out = cx_test(cx_call_arg(call, 1)),
    *in = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  struct cx_proc *p = cx_proc_new(s->cx);
  
  struct cx_file
    *in_file = (in->type == s->cx->nil_type) ? NULL : in->as_file,
    *out_file = (out->type == s->cx->nil_type) ? NULL : out->as_file,
    *error_file = (error->type == s->cx->nil_type) ? NULL : error->as_file;
  
  switch(cx_proc_fork(p, in_file, out_file, error_file)) {
  case 0:
    cx_box_init(cx_push(s), s->cx->nil_type);
    cx_proc_deref(p);
    break;
  case -1:
    cx_proc_deref(p);
    return false;
  default:
    cx_box_init(cx_push(s), s->cx->proc_type)->as_proc = p;
    break;
  }

  return true;
}

static bool exec_imp(struct cx_call *call) {
  struct cx_box
    *argsv = cx_test(cx_call_arg(call, 1)),
    *cmd = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  struct cx_stack *args = argsv->as_ptr;
  char *as[args->imp.count+2];
  as[0] = cmd->as_str->data;
  as[args->imp.count+1] = NULL;
  char **asp = as+1;
  
  cx_do_vec(&args->imp, struct cx_box, a) {
    if (a->type != s->cx->str_type) {
      cx_error(s->cx, s->cx->row, s->cx->col, "Invalid argument: %s", a->type->id);
      return false;
    }

    *asp++ = a->as_str->data;
  }

  execvp(cmd->as_str->data, as);
  cx_error(s->cx, s->cx->row, s->cx->col, "Failed executing command: %d", errno);
  return false;
}

static bool no_exec_imp(struct cx_call *call) {
  struct cx_box *f = cx_test(cx_call_arg(call, 0));
  return cx_noexec(call->scope->cx, f->as_file->fd);
}

static bool in_imp(struct cx_call *call) {
  struct cx_proc *p = cx_test(cx_call_arg(call, 0))->as_proc;
  struct cx_scope *s = call->scope;
  if (!p->in && p->in_fd != -1) { p->in = cx_file_new(s->cx, p->in_fd, "w", NULL); }

  if (p->in) {
    cx_box_init(cx_push(s), s->cx->wfile_type)->as_file = cx_file_ref(p->in);
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }
  
  return true;
}

static bool out_imp(struct cx_call *call) {
  struct cx_proc *p = cx_test(cx_call_arg(call, 0))->as_proc;
  struct cx_scope *s = call->scope;
  
  if (!p->out && p->out_fd != -1) {
    p->out = cx_file_new(s->cx, p->out_fd, "r", NULL);
  }

  if (p->out) {
    cx_box_init(cx_push(s), s->cx->rfile_type)->as_file = cx_file_ref(p->out);
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }
  
  return true;
}

static bool error_imp(struct cx_call *call) {
  struct cx_proc *p = cx_test(cx_call_arg(call, 0))->as_proc;
  struct cx_scope *s = call->scope;
  
  if (!p->error && p->error_fd != -1) {
    p->error = cx_file_new(s->cx, p->error_fd, "r", NULL);
  }

  if (p->error) {
    cx_box_init(cx_push(s), s->cx->rfile_type)->as_file = cx_file_ref(p->error);
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }
  
  return true;
}

static bool wait_imp(struct cx_call *call) {
  struct cx_box
    *ms = cx_test(cx_call_arg(call, 1)),
    *p = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  struct cx_box status;
  
  if (!cx_proc_wait(p->as_proc, ms->as_int, &status)) {
    cx_box_init(cx_push(s), s->cx->nil_type);
    return true;
  }
  
  *cx_push(s) = status;
  return true;
}

static bool kill_imp(struct cx_call *call) {
  struct cx_box
    *ms = cx_test(cx_call_arg(call, 1)),
    *p = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  struct cx_box status;
  
  if (!cx_proc_kill(p->as_proc, ms->as_int, &status)) {
    cx_box_init(cx_push(s), s->cx->nil_type);
    return true;
  }
  
  *cx_push(s) = status;
  return true;
}

static bool exit_imp(struct cx_call *call) {
  struct cx_box *status = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  
  exit(status->as_int);
  cx_error(s->cx, s->cx->row, s->cx->col, "Failed exiting");
  return false;
}

cx_lib(cx_init_proc, "cx/proc") {    
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "#nil", "Cmp", "Opt") ||
      !cx_use(cx, "cx/cond", "else") ||
      !cx_use(cx, "cx/io", "File", "RFile", "WFile") ||
      !cx_use(cx, "cx/stack", "%", "Stack")) {
    return false;
  }

  cx->proc_type = cx_init_proc_type(lib);
  
  cx_add_cfunc(lib, "fork",
	       cx_args(cx_arg("in", cx_type_get(cx->opt_type, cx->rfile_type)),
		       cx_arg("out", cx_type_get(cx->opt_type, cx->wfile_type)),
		       cx_arg("error", cx_type_get(cx->opt_type, cx->wfile_type))),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->proc_type))),
	       fork_imp);

  cx_add_cfunc(lib, "exec",
	       cx_args(cx_arg("cmd", cx->str_type), cx_arg("args", cx->stack_type)),
	       cx_args(),
	       exec_imp);

  cx_add_cfunc(lib, "no-exec",
	       cx_args(cx_arg("f", cx->file_type)),
	       cx_args(),
	       no_exec_imp);

  cx_add_cfunc(lib, "in",
	       cx_args(cx_arg("p", cx->proc_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->wfile_type))),
	       in_imp);

  cx_add_cfunc(lib, "out",
	       cx_args(cx_arg("p", cx->proc_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->rfile_type))),
	       out_imp);

  cx_add_cfunc(lib, "error",
	       cx_args(cx_arg("p", cx->proc_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->rfile_type))),
	       error_imp);

  cx_add_cfunc(lib, "wait",
	       cx_args(cx_arg("p", cx->proc_type), cx_arg("ms", cx->int_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->int_type))),
	       wait_imp);

  cx_add_cfunc(lib, "kill",
	       cx_args(cx_arg("p", cx->proc_type), cx_arg("ms", cx->int_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->int_type))),
	       kill_imp);

  cx_add_cxfunc(lib, "popen",
	       cx_args(cx_arg("cmd", cx->str_type), cx_arg("args", cx->stack_type)),
	       cx_args(cx_arg(NULL, cx->proc_type)),
	       "#nil #nil #nil fork % {$cmd $args exec} else");

  cx_add_cfunc(lib, "exit",
	       cx_args(cx_arg("status", cx->int_type)),
	       cx_args(),
	       exit_imp);

  return true;
}
