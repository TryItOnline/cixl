#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/cx.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/str.h"
#include "cixl/types/sym.h"

struct cx_sym *cx_sym_init(struct cx_sym *sym, const char *id, size_t tag) {
  sym->id = strdup(id);
  sym->emit_id = cx_emit_id("sym", id);
  sym->tag = tag;
  return sym;
}

struct cx_sym *cx_sym_deinit(struct cx_sym *sym) {
  free(sym->id);
  free(sym->emit_id);
  return sym;
}

enum cx_cmp cx_cmp_sym(const void *x, const void *y) {
  const struct cx_sym *xs = x, *ys = y;
  if (xs->tag == ys->tag) { return CX_CMP_EQ; }
  return (strcmp(xs->id, ys->id) < 0) ? CX_CMP_LT : CX_CMP_GT;
}

static bool sym_imp(struct cx_scope *scope) {
  struct cx_box s = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope),
	      scope->cx->sym_type)->as_sym = cx_sym(scope->cx, s.as_str->data);
  cx_box_deinit(&s);
  return true;
}

static bool str_imp(struct cx_scope *scope) {
  struct cx_sym s = cx_test(cx_pop(scope, false))->as_sym;
  cx_box_init(cx_push(scope), scope->cx->str_type)->as_str = cx_str_new(s.id);
  return true;
}

static void new_imp(struct cx_box *out) {
  struct cx *cx = out->type->cx;
  char *id = cx_fmt("S%zd", cx->next_sym_tag);
  out->as_sym = cx_sym(cx, id);
  free(id);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_sym.tag == y->as_sym.tag;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "`%s", v->as_sym.id);
}

static void print_imp(struct cx_box *v, FILE *out) {
  fputs(v->as_sym.id, out);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  fprintf(out,
	  CX_TAB "cx_box_init(%s, cx->sym_type)->as_sym "
	  "= %s;\n",
	  exp, v->as_sym.emit_id);
  return true;
}

struct cx_type *cx_init_sym_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Sym", cx->any_type);
  t->new = new_imp;
  t->equid = equid_imp;
  t->write = dump_imp;
  t->dump = dump_imp;
  t->print = print_imp;
  t->emit = emit_imp;
  
  cx_add_cfunc(cx, "sym",
	       cx_args(cx_arg("id", cx->str_type)),
	       cx_rets(cx_ret(t)),
	       sym_imp);
  
  cx_add_cfunc(cx, "str",
	       cx_args(cx_arg("s", t)),
	       cx_rets(cx_ret(cx->str_type)),
	       str_imp);
  
  return t;
}
