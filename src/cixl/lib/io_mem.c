#include <string.h>
#include <inttypes.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/mfile.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/lib.h"
#include "cixl/lib/io_mem.h"

static bool str_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box f = *cx_test(cx_pop(scope, false));
  struct cx_mfile_ref *mf = cx_baseof(f.as_file, struct cx_mfile_ref, file);
  fflush(mf->file._ptr);
  cx_box_init(cx_push(scope), cx->str_type)->as_str = cx_str_new(mf->data);
  cx_box_deinit(&f);
  return true;
}

cx_lib(cx_init_io_mem, "cx/io/mem") { 
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "Str") ||
      !cx_use(cx, "cx/io", "RWFile") ||
      !cx_use(cx, "cx/type", "new")) {
    return false;
  }

  cx->mfile_type = cx_init_mfile_type(lib);

  cx_add_cfunc(lib, "str",
	       cx_args(cx_arg("f", cx->mfile_type)),
	       cx_args(cx_arg(NULL, cx->str_type)),
	       str_imp);
  
  return true;
}
