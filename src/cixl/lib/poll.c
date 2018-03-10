#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/file.h"
#include "cixl/lib.h"
#include "cixl/lib/poll.h"
#include "cixl/poll.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static bool on_read_imp(struct cx_scope *scope) {
  struct cx_box
    a = *cx_test(cx_pop(scope, false)),
    f = *cx_test(cx_pop(scope, false)),
    p = *cx_test(cx_pop(scope, false));
  
  bool ok = cx_poll_read(p.as_poll, f.as_file, &a);

  cx_box_deinit(&a);
  cx_box_deinit(&f);
  cx_box_deinit(&p);
  return ok;
}

static bool delete_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;

  struct cx_box
    f = *cx_test(cx_pop(scope, false)),
    p = *cx_test(cx_pop(scope, false));

  bool ok = cx_poll_delete(p.as_poll, f.as_file);
  if (!ok) { cx_error(cx, cx->row, cx->col, "File is not polled"); }
  cx_box_deinit(&f);
  cx_box_deinit(&p);
  return ok;
}

static bool wait_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;

  struct cx_box
    ms = *cx_test(cx_pop(scope, false)),
    p = *cx_test(cx_pop(scope, false));

  int n = cx_poll_wait(p.as_poll, ms.as_int, scope);
  cx_box_deinit(&p);

  if (n == -1) {
    cx_error(cx, cx->row, cx->col, "Failed polling");
    return false;
  }
  
  cx_box_init(cx_push(scope), cx->int_type)->as_int = n;
  return true;
}

static bool len_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box p = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = p.as_poll->files.members.count;
  return true;
}

cx_lib(cx_init_poll, "cx/io/poll") {    
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Int") ||
      !cx_use(cx, "cx/io", "File", "RFile")) {
    return false;
  }

  cx->poll_type = cx_init_poll_type(lib);
  
  cx_add_cfunc(lib, "on-read",
	       cx_args(cx_arg("p", cx->poll_type),
		       cx_arg("f", cx->rfile_type),
		       cx_arg("a", cx->any_type)),
	       cx_args(),
	       on_read_imp);

  cx_add_cfunc(lib, "delete",
	       cx_args(cx_arg("p", cx->poll_type), cx_arg("f", cx->file_type)),
	       cx_args(),
	       delete_imp);

  cx_add_cfunc(lib, "wait",
	       cx_args(cx_arg("p", cx->poll_type), cx_arg("ms", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       wait_imp);

  cx_add_cfunc(lib, "len",
	       cx_args(cx_arg("p", cx->poll_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       len_imp);

  return true;
}
