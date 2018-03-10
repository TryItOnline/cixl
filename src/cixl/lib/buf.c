#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/file.h"
#include "cixl/iter.h"
#include "cixl/lib.h"
#include "cixl/lib/buf.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static bool clear_imp(struct cx_scope *scope) {
  struct cx_box b = *cx_test(cx_pop(scope, false));
  cx_vec_clear(&b.as_buf->data);
  cx_box_deinit(&b);
  return true;
}

static bool len_imp(struct cx_scope *scope) {
  struct cx_box b = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = b.as_buf->data.count;
  cx_box_deinit(&b);
  return true;
}

cx_lib(cx_init_buf, "cx/buf") {    
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Int")) {
    return false;
  }

  cx->buf_type = cx_init_buf_type(lib);

  cx_add_cfunc(lib, "clear",
	       cx_args(cx_arg("b", cx->buf_type)),
	       cx_args(),
	       clear_imp);

  cx_add_cfunc(lib, "len",
	       cx_args(cx_arg("b", cx->buf_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       len_imp);

  return true;
}
