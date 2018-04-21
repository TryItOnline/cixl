#include <string.h>
#include <inttypes.h>

#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/lib/bin.h"
#include "cixl/mfile.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static bool compile_imp(struct cx_call *call) {  
  struct cx_box
    *in = cx_test(cx_call_arg(call, 1)),
    *out = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = cx_parse_str(s->cx, in->as_str->data, &toks);
  if (!ok) { goto exit; }
  
  struct cx_bin *bin = out->as_ptr;

  if (!(ok = cx_compile(s->cx, cx_vec_start(&toks), cx_vec_end(&toks), bin))) {
    goto exit;
  }
 exit: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

static bool emit_imp(struct cx_call *call) {
  struct cx_scope *s = call->scope;
  struct cx_box *in = cx_test(cx_call_arg(call, 0));
  struct cx_bin *bin = in->as_ptr;
  struct cx_mfile out;
  cx_mfile_open(&out);  
  bool ok = cx_emit(bin, out.stream, s->cx);
  if (!ok) { goto exit; }
  fflush(out.stream);
  cx_box_init(cx_push(s), s->cx->str_type)->as_str = cx_str_new(out.data, out.size);
  ok = true;
 exit:
  cx_mfile_close(&out);
  free(out.data);
  return ok;
}

cx_lib(cx_init_bin, "cx/bin") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Str")) {
    return false;
  }

  cx->bin_type = cx_init_bin_type(lib);

  cx_add_cfunc(lib, "compile",
	       cx_args(cx_arg("out", cx->bin_type), cx_arg("in", cx->str_type)),
	       cx_args(),
	       compile_imp);
    
  cx_add_cfunc(lib, "emit",
	       cx_args(cx_arg("in", cx->bin_type)),
	       cx_args(cx_arg(NULL, cx->str_type)),
	       emit_imp);

  return true;
}
