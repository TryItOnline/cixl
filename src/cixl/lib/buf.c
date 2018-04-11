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
#include "cixl/mfile.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static bool clear_imp(struct cx_scope *scope) {
  struct cx_box in = *cx_test(cx_pop(scope, false));
  cx_buf_clear(cx_baseof(in.as_file, struct cx_buf, file));
  cx_box_deinit(&in);
  return true;
}

static bool len_imp(struct cx_scope *scope) {
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_buf *b = cx_baseof(in.as_file, struct cx_buf, file);
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = cx_buf_len(b);
  cx_box_deinit(&in);
  return true;
}

static bool str_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_buf *b = cx_baseof(in.as_file, struct cx_buf, file);
  fflush(b->file._ptr);
  cx_box_init(cx_push(scope), cx->str_type)->as_str =
    cx_str_new(b->data+b->pos, b->len-b->pos);
  cx_box_deinit(&in);
  return true;
}

static bool read_bytes_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  bool ok = false;

  struct cx_box
    nbytes = *cx_test(cx_pop(scope, false)),
    in = *cx_test(cx_pop(scope, false)),
    buf = *cx_test(cx_pop(scope, false));

  struct cx_buf *b = cx_baseof(buf.as_file, struct cx_buf, file);
  size_t offs = ftell(b->file._ptr);
  fseek(b->file._ptr, nbytes.as_int, SEEK_CUR);
  fflush(b->file._ptr);
  int rbytes = read(in.as_file->fd, b->data+offs, nbytes.as_int);

  if (!rbytes ||
      (rbytes == -1 && (errno == ECONNREFUSED || errno == ECONNRESET))) {
    cx_box_init(cx_push(scope), cx->nil_type);
    fseek(b->file._ptr, offs, SEEK_SET);
    ok = true;
    goto exit;
  }

  if (rbytes == -1 && errno != EAGAIN) {
    cx_error(cx, cx->row, cx->col, "Failed reading: %d", errno);
    goto exit;
  }

  cx_box_init(cx_push(scope), cx->int_type)->as_int = rbytes;
  ok = true;
 exit:
  cx_box_deinit(&buf);
  cx_box_deinit(&in);
  return ok;
}

static bool write_bytes_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  bool ok = false;

  struct cx_box
    out = *cx_test(cx_pop(scope, false)),
    buf = *cx_test(cx_pop(scope, false));

  struct cx_buf *b = cx_baseof(buf.as_file, struct cx_buf, file);  
  int wbytes = write(out.as_file->fd, b->data+b->pos, b->len-b->pos);

  if (wbytes == -1 && errno != EAGAIN) {
    cx_error(cx, cx->row, cx->col, "Failed writing: %d", errno);
    goto exit;
  }

  b->pos += wbytes;
  cx_box_init(cx_push(scope), cx->bool_type)->as_bool = b->pos == b->len;
  if (b->pos == b->len) { cx_buf_clear(b); }
  ok = true;
 exit:
  cx_box_deinit(&buf);
  cx_box_deinit(&out);
  return ok;
}

cx_lib(cx_init_buf, "cx/io/buf") {    
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "Int", "Opt", "Str") ||
      !cx_use(cx, "cx/io", "RFile", "WFile") ||
      !cx_use(cx, "cx/iter", "for") ||
      !cx_use(cx, "cx/stack", "~") ||
      !cx_use(cx, "cx/type", "new")) {
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

  cx_add_cfunc(lib, "str",
	       cx_args(cx_arg("b", cx->buf_type)),
	       cx_args(cx_arg(NULL, cx->str_type)),
	       str_imp);

  cx_add_cfunc(lib, "read-bytes",
	       cx_args(cx_arg("buf", cx->buf_type),
		       cx_arg("in", cx->rfile_type),
		       cx_arg("nbytes", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       read_bytes_imp);
      
  cx_add_cfunc(lib, "write-bytes",
	       cx_args(cx_arg("buf", cx->buf_type),
		       cx_arg("out", cx->wfile_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       write_bytes_imp);

  return true;
}
