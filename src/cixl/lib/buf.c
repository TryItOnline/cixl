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

static bool push_char_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    b = *cx_test(cx_pop(scope, false));

  cx_buf_push_char(b.as_buf, v.as_char);
  cx_box_deinit(&v);
  cx_box_deinit(&b);
  return true;
}

static bool push_str_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    b = *cx_test(cx_pop(scope, false));

  cx_buf_push_str(b.as_buf, v.as_str->data);
  cx_box_deinit(&v);
  cx_box_deinit(&b);
  return true;
}

static bool push_sym_imp(struct cx_scope *scope) {
  struct cx_box
    v = *cx_test(cx_pop(scope, false)),
    b = *cx_test(cx_pop(scope, false));

  cx_buf_push_str(b.as_buf, v.as_sym.id);
  cx_box_deinit(&b);
  return true;
}

static bool clear_imp(struct cx_scope *scope) {
  struct cx_box b = *cx_test(cx_pop(scope, false));
  cx_buf_clear(b.as_buf);
  cx_box_deinit(&b);
  return true;
}

static bool len_imp(struct cx_scope *scope) {
  struct cx_box b = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = b.as_buf->data.count;
  cx_box_deinit(&b);
  return true;
}

static bool read_bytes_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  bool ok = false;

  struct cx_box
    nbytes = *cx_test(cx_pop(scope, false)),
    out = *cx_test(cx_pop(scope, false)),
    in = *cx_test(cx_pop(scope, false));

  struct cx_buf *o = out.as_buf;
  cx_vec_grow(&o->data, o->data.count+nbytes.as_int);  
  int rbytes = read(in.as_file->fd, o->data.items, nbytes.as_int);

  if (!rbytes || (rbytes == -1 && errno == ECONNREFUSED)) {
    cx_box_init(cx_push(scope), cx->nil_type);
    ok = true;
    goto exit;
  }

  if (rbytes == -1 && errno == EAGAIN) { rbytes = 0; }

  if (rbytes == -1) {
    cx_error(cx, cx->row, cx->col, "Failed reading: %d", errno);
    goto exit;
  }

  o->data.count += rbytes;
  cx_box_init(cx_push(scope), cx->int_type)->as_int = rbytes;
  ok = true;
 exit:
  cx_box_deinit(&out);
  cx_box_deinit(&in);
  return ok;
}

static bool write_bytes_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  bool ok = false;

  struct cx_box
    buf = *cx_test(cx_pop(scope, false)),
    out = *cx_test(cx_pop(scope, false));

  struct cx_buf *b = buf.as_buf;  
  int wbytes = write(out.as_file->fd, cx_buf_ptr(b), cx_buf_len(b));

  if (wbytes == -1 && errno != EAGAIN) {
    cx_error(cx, cx->row, cx->col, "Failed writing: %d", errno);
    goto exit;
  }

  b->pos += wbytes;
  if (b->pos == b->data.count) { cx_buf_clear(b); }
  ok = true;
 exit:
  cx_box_deinit(&buf);
  cx_box_deinit(&out);
  return ok;
}


cx_lib(cx_init_buf, "cx/buf") {    
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Int", "Opt", "Stack", "Str", "Sym") ||
      !cx_use(cx, "cx/io", "RFile", "WFile") ||
      !cx_use(cx, "cx/iter", "for") ||
      !cx_use(cx, "cx/stack", "~")) {
    return false;
  }

  cx->buf_type = cx_init_buf_type(lib);

  cx_add_cfunc(lib, "push",
	       cx_args(cx_arg("b", cx->buf_type), cx_arg("v", cx->char_type)),
	       cx_args(),
	       push_char_imp);

  cx_add_cfunc(lib, "push",
	       cx_args(cx_arg("b", cx->buf_type), cx_arg("v", cx->str_type)),
	       cx_args(),
	       push_str_imp);

  cx_add_cfunc(lib, "push",
	       cx_args(cx_arg("b", cx->buf_type), cx_arg("v", cx->sym_type)),
	       cx_args(),
	       push_sym_imp);

  cx_add_cxfunc(lib, "push",
		cx_args(cx_arg("b", cx->buf_type), cx_arg("v", cx->stack_type)),
		cx_args(),
		"$v {$b ~ push} for");

  cx_add_cfunc(lib, "clear",
	       cx_args(cx_arg("b", cx->buf_type)),
	       cx_args(),
	       clear_imp);

  cx_add_cfunc(lib, "len",
	       cx_args(cx_arg("b", cx->buf_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       len_imp);

  cx_add_cfunc(lib, "read-bytes",
	       cx_args(cx_arg("in", cx->rfile_type),
		       cx_arg("out", cx->buf_type),
		       cx_arg("nbytes", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       read_bytes_imp);
      
  cx_add_cfunc(lib, "write-bytes",
	       cx_args(cx_arg("out", cx->wfile_type),
		       cx_arg("buf", cx->buf_type)),
	       cx_args(),
	       write_bytes_imp);

  return true;
}
