#include "cixl/cx.h"
#include "cixl/lib.h"
#include "cixl/opt.h"
#include "cixl/type.h"

static void type_init_imp(struct cx_type *t, int nargs, struct cx_type *args[]) {
  struct cx *cx = t->lib->cx;
  
  if (cx->nil_type) {
    cx_derive(args[0], t);
    cx_derive(cx->nil_type, t);
  }
}

struct cx_type *cx_init_opt_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Opt", cx->opt_type);
  cx_type_push_args(t, t);
  t->meta = CX_TYPE_ID;
  t->type_init = type_init_imp;
  return t;
}
