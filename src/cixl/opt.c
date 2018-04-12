#include "cixl/cx.h"
#include "cixl/lib.h"
#include "cixl/opt.h"
#include "cixl/type.h"

struct cx_type *type_get_imp(struct cx_type *t,
			     const char *id,
			     int nargs, struct cx_type *args[]) {
  struct cx *cx = t->lib->cx;
  struct cx_type *tt = cx_type_new(t->lib, id);
  
  if (cx->nil_type) {
    cx_derive(args[0], tt);
    cx_derive(cx->nil_type, tt);
  }

  return tt;
}

struct cx_type *cx_init_opt_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Opt", cx->opt_type);
  cx_type_push_args(t, t);
  t->trait = true;
  t->type_get = type_get_imp;
  return t;
}
