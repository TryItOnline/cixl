#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/file.h"
#include "cixl/lib.h"
#include "cixl/lib/queue.h"
#include "cixl/queue.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static bool push_imp(struct cx_scope *scope) {
  struct cx_box
    val = *cx_test(cx_pop(scope, false)),
    q = *cx_test(cx_pop(scope, false));

  cx_queue_push(q.as_queue, &val);

  cx_box_deinit(&val);
  cx_box_deinit(&q);
  return true;
}

static bool pop_imp(struct cx_scope *scope) {
  struct cx_box
    act = *cx_test(cx_pop(scope, false)),
    q = *cx_test(cx_pop(scope, false));
  
  cx_queue_pop(q.as_queue, &act);
  
  cx_box_deinit(&act);
  cx_box_deinit(&q);
  return true;
}

static bool poll_imp(struct cx_scope *scope) {
  struct cx_box
    p = *cx_test(cx_pop(scope, false)),
    q = *cx_test(cx_pop(scope, false));
  
  cx_queue_poll(q.as_queue, p.as_poll);
  
  cx_box_deinit(&p);
  cx_box_deinit(&q);
  return true;
}

static bool len_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box q = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = cx_queue_len(q.as_queue);
  cx_box_deinit(&q);
  return true;
}

cx_lib(cx_init_queue, "cx/io/queue") {    
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Int") ||
      !cx_use(cx, "cx/io/poll", "Poll")) {
    return false;
  }

  cx->queue_type = cx_init_queue_type(lib);

  cx_add_cfunc(lib, "push",
	       cx_args(cx_arg("q", cx->queue_type), cx_arg("val", cx->any_type)),
	       cx_args(),
	       push_imp);

  cx_add_cfunc(lib, "on-pop",
	       cx_args(cx_arg("q", cx->queue_type), cx_arg("act", cx->any_type)),
	       cx_args(),
	       pop_imp);

  cx_add_cfunc(lib, "poll",
	       cx_args(cx_arg("q", cx->queue_type),
		       cx_arg("p", cx->poll_type)),
	       cx_args(),
	       poll_imp);

  cx_add_cfunc(lib, "len",
	       cx_args(cx_arg("q", cx->queue_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       len_imp);

  return true;
}
