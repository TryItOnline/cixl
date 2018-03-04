#include <string.h>
#include <inttypes.h>

#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/bin.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static bool compile_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    in = *cx_test(cx_pop(scope, false)),
    out = *cx_test(cx_pop(scope, false));

  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = cx_parse_str(cx, in.as_str->data, &toks);
  if (!ok) { goto exit; }
  
  struct cx_bin *bin = out.as_ptr;

  if (!(ok = cx_compile(cx, cx_vec_start(&toks), cx_vec_end(&toks), bin))) {
    goto exit;
  }
 exit:
  cx_box_deinit(&in);
  cx_box_deinit(&out);
  cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&toks);
  return ok;
}

static bool emit_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_bin *bin = in.as_ptr;
  struct cx_buf out;
  cx_buf_open(&out);  
  bool ok = cx_emit(bin, out.stream, cx);
  if (!ok) { goto exit; }
  fflush(out.stream);
  cx_box_init(cx_push(scope), cx->str_type)->as_str = cx_str_new(out.data);
  ok = true;
 exit:
  cx_box_deinit(&in);
  cx_buf_close(&out);
  free(out.data);
  return ok;
}

cx_lib(cx_init_bin, "cx/bin", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");
    cx_use(cx, "cx/bin/types");
    cx_use(cx, "cx/str/types");

    cx_add_cfunc(lib, "compile",
		 cx_args(cx_arg("out", cx->bin_type), cx_arg("in", cx->str_type)),
		 cx_args(),
		 compile_imp);
    
    cx_add_cfunc(lib, "emit",
		 cx_args(cx_arg("in", cx->bin_type)),
		 cx_args(cx_arg(NULL, cx->str_type)),
		 emit_imp);
  })

cx_lib(cx_init_bin_types, "cx/bin/types", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");
    cx->bin_type = cx_init_bin_type(lib);
  });
