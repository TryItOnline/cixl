#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  struct cx_fimp *imp = value->as_ptr;
  
  if (!cx_fimp_match(imp, &scope->stack)) {
    struct cx *cx = scope->cx;
    cx_error(cx, cx->row, cx->col, "Func not applicable: '%s'", imp->func->id);
    return -1;
  }

  return cx_fimp_call(imp, scope);
}

static void fprint_imp(struct cx_box *value, FILE *out) {
  struct cx_fimp *imp = value->as_ptr;
  fprintf(out, "Fimp(%s", imp->func->id);
  cx_do_vec(&imp->args, struct cx_func_arg, a) {
    if (a->type) {
      fprintf(out, " %s", a->type->id);
    } else {
      fprintf(out, " %d", a->narg);      
    }
  }
  fputc(')', out);
}

struct cx_type *cx_init_fimp_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Fimp", cx->any_type);
  t->equid = equid_imp;
  t->call = call_imp;
  t->fprint = fprint_imp;
  return t;
}
