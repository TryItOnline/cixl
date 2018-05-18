#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "cixl/arg.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/task.h"
#include "cixl/sched.h"
#include "cixl/scope.h"
#include "cixl/task.h"

static bool resched_imp(struct cx_call *call) {
  struct cx_scope *s = call->scope;
  return cx_task_resched(cx_test(s->cx->task), s);
}

static bool run_imp(struct cx_call *call) {
  struct cx_sched *s = cx_test(cx_call_arg(call, 0))->as_sched;
  return cx_sched_run(s, call->scope);
}

cx_lib(cx_init_task, "cx/task") {    
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "Sink") ||
      !cx_use(cx, "cx/type", "new")) {
    return false;
  }

  cx->sched_type = cx_init_sched_type(lib);
  
  cx_add_cfunc(lib, "resched",
	       cx_args(),
	       cx_args(),
	       resched_imp);

  cx_add_cfunc(lib, "run",
	       cx_args(cx_arg("s", cx->sched_type)),
	       cx_args(),
	       run_imp);

  return true;
}
