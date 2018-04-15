#include <inttypes.h>

#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/call_iter.h"
#include "cixl/cx.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

struct cx_fimp *cx_fimp_init(struct cx_fimp *imp,
			     struct cx_lib *lib,
			     struct cx_func *func,
			     char *id) {
  imp->lib = lib;
  imp->func = func;
  imp->id = id;
  imp->emit_id = cx_emit_id(func->emit_id, id);
  imp->ptr = NULL;
  imp->bin = NULL;
  imp->scope = NULL;
  imp->init = true;
  
  cx_vec_init(&imp->args, sizeof(struct cx_arg));
  cx_vec_init(&imp->rets, sizeof(struct cx_arg));
  cx_vec_init(&imp->toks, sizeof(struct cx_tok));
  return imp;
}

struct cx_fimp *cx_fimp_deinit(struct cx_fimp *imp) {
  free(imp->id);
  free(imp->emit_id);
  
  cx_do_vec(&imp->args, struct cx_arg, a) { cx_arg_deinit(a); }
  cx_vec_deinit(&imp->args);

  cx_do_vec(&imp->rets, struct cx_arg, r) { cx_arg_deinit(r); }
  cx_vec_deinit(&imp->rets);
  
  cx_do_vec(&imp->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&imp->toks);

  if (imp->bin) { cx_bin_deref(imp->bin); }
  if (imp->scope) { cx_scope_deref(imp->scope); }
  return imp;
}

ssize_t cx_fimp_score(struct cx_fimp *imp, struct cx_scope *scope, ssize_t max) {
  struct cx_vec *stack = &scope->stack;
  
  if (stack->count < imp->args.count) { return -1; }
  if (!imp->args.count) { return 0; }
  
  struct cx_arg *i = (struct cx_arg *)cx_vec_peek(&imp->args, 0);
  struct cx_box *j = (struct cx_box *)cx_vec_peek(stack, 0);
  size_t score = 0;

  struct cx_type *get_imp_arg(int i) {
    return (i < imp->func->nargs)
      ? ((struct cx_arg *)cx_vec_get(&imp->args, i))->type
      : NULL;
  }

  struct cx_type *get_stack(int i) {
    return (i < imp->func->nargs)
      ? ((struct cx_box *)cx_vec_get(stack, stack->count-imp->func->nargs+i))->type
      : NULL;
  }

  for (; i >= (struct cx_arg *)imp->args.items &&
	 j >= (struct cx_box *)stack->items;
       i--, j--) {
    if (i->arg_type == CX_VARG) {
      if (j->type != i->value.type || !cx_eqval(&i->value, j)) { return -1; }
      continue;
    }
    
    struct cx_type *t = cx_resolve_arg_refs(i->type, get_imp_arg, get_stack);
    if (!t) { return -1; }
    score += cx_abs((ssize_t)j->type->level - t->level);
    if (max > -1 && score >= max) { return -1; }
    if (!cx_is(j->type, t)) { return -1; }
  }

  return score;
}

bool cx_fimp_match(struct cx_fimp *imp, struct cx_scope *scope) {
  return cx_fimp_score(imp, scope, -1) > -1;
}

static bool compile(struct cx_fimp *imp, size_t tok_idx, struct cx_bin *out) {
  struct cx *cx = imp->func->lib->cx;
  size_t start_pc = out->ops.count;
  
  struct cx_op *op = cx_op_init(out, CX_OBEGIN(), tok_idx);
  op->as_begin.child = false;
  op->as_begin.fimp = imp;

  if (imp->args.count) {
    cx_op_init(out, CX_OPUTARGS(), tok_idx)->as_putargs.imp = imp;
  }

  if (imp->toks.count) {
    cx_push_lib(cx, imp->lib);
    bool ok = cx_compile(cx, cx_vec_start(&imp->toks), cx_vec_end(&imp->toks), out);
    cx_pop_lib(cx);
    
    if (!ok) {
      cx_error(cx, cx->row, cx->col, "Failed compiling fimp");
      return false;
    }
  }
  
  op = cx_op_init(out, CX_ORETURN(), tok_idx);
  op->as_return.imp = imp;
  op->as_return.pc = start_pc;  
  return true;
}

struct cx_bin_fimp *cx_fimp_inline(struct cx_fimp *imp,
				   size_t tok_idx,
				   struct cx_bin *out,
				   struct cx *cx) {
  struct cx_bin_fimp *bf = cx_set_get(&out->fimps, &imp);

  if (!bf) {
    bf = cx_test(cx_set_insert(&out->fimps, &imp));
    bf->imp = imp;
    bf->start_pc = out->ops.count+1;
    bf->nops = -1;
    struct cx_op *op = cx_op_init(out, CX_OFIMP(), tok_idx);
    op->as_fimp.imp = imp;
    op->as_fimp.init = imp->init;
    if (!compile(imp, tok_idx, out)) { return NULL; }
    bf = cx_test(cx_set_get(&out->fimps, &imp));
    bf->nops = out->ops.count - bf->start_pc;
  }
  
  return bf;
}

bool cx_fimp_call(struct cx_fimp *imp, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp);

  if (imp->ptr) {    
    size_t lib_count = cx->libs.count;
    if (*cx->lib != imp->lib) { cx_push_lib(cx, imp->lib); }
    bool ok = imp->ptr(scope);
    while (cx->libs.count > lib_count) { cx_pop_lib(cx); }
    cx_call_deinit(cx_vec_pop(&cx->calls));
    return ok;
  }

  struct cx_bin *bin = imp->bin ? imp->bin : cx->bin;
  struct cx_bin_fimp *bimp = cx_set_get(&bin->fimps, &imp);

  if (!bimp) {
    cx_test(!imp->bin);
    bin = imp->bin = cx_bin_new();
    bimp = cx_fimp_inline(imp, 0, bin, cx);
  }

  return cx_eval(bin,
		 bimp->start_pc,
		 bimp->start_pc+bimp->nops,
		 cx);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  struct cx_fimp *imp = value->as_ptr;

  if (scope->safe && !cx_fimp_match(imp, scope)) {
    struct cx *cx = scope->cx;
    cx_error(cx, cx->row, cx->col, "Func not applicable: '%s'", imp->func->id);
    return false;
  }

  return cx_fimp_call(imp, scope);
}

static void iter_imp(struct cx_box *in, struct cx_box *out) {
  struct cx *cx = in->type->lib->cx;
  cx_box_init(out, cx->iter_type)->as_iter = cx_call_iter_new(in);
}

static void write_imp(struct cx_box *value, FILE *out) {
  struct cx_fimp *imp = value->as_ptr;
  fprintf(out, "&%s<%s>", imp->func->id, imp->id);
}

static void dump_imp(struct cx_box *value, FILE *out) {
  struct cx_fimp *imp = value->as_ptr;
  fprintf(out, "Fimp(%s", imp->func->id);

  if (imp->func->nargs) {
    fprintf(out, " %s)",  imp->id);
  } else {
    fputc(')', out);
  }
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  struct cx_func *fimp = v->as_ptr;
  
  fprintf(out,
	  "cx_box_init(%s, cx->fimp_type)->as_ptr = %s();\n",
	  exp, fimp->emit_id);

  return true;
}

struct cx_type *cx_init_fimp_type(struct cx_lib *lib) {
  struct cx_type *t = cx_add_type(lib, "Fimp", lib->cx->seq_type);
  t->equid = equid_imp;
  t->call = call_imp;
  t->iter = iter_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  t->emit = emit_imp;
  return t;
}
