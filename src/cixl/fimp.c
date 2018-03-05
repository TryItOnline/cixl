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
			     struct cx_func *func,
			     char *id) {
  imp->func = func;
  imp->id = id;
  imp->emit_id = cx_emit_id(func->emit_id, id);
  imp->ptr = NULL;
  imp->bin = NULL;
  imp->start_pc = imp->nops = 0;
  imp->scope = NULL;
  
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

ssize_t cx_fimp_score(struct cx_fimp *imp, struct cx_scope *scope) {
  struct cx_vec *stack = &scope->stack;
  if (stack->count < imp->args.count) { return -1; }
  if (!imp->args.count) { return 0; }
  
  struct cx_arg *i = (struct cx_arg *)cx_vec_peek(&imp->args, 0);
  struct cx_box *j = (struct cx_box *)cx_vec_peek(stack, 0);
  size_t score = 0;
  
  for (; i >= (struct cx_arg *)imp->args.items &&
	 j >= (struct cx_box *)stack->items;
       i--, j--) {
    if (i->arg_type == CX_VARG) {
      if (!cx_eqval(&i->value, j)) { return -1; }
      continue;
    }
    
    struct cx_type *t = NULL;

    switch (i->arg_type) {
    case CX_ARG:
      t = i->type;
      break;
    case CX_NARG:
      t = ((struct cx_box *)cx_vec_get(stack,
				       stack->count-imp->args.count+i->narg))->type;
      break;
    default:
      break;
    }
    
    if (!cx_is(j->type, cx_test(t))) { return -1; }
    score += cx_abs((ssize_t)j->type->level - t->level);
  }

  return score;
}

static bool compile(struct cx_fimp *imp, size_t tok_idx, struct cx_bin *out) {
  struct cx *cx = imp->func->cx;
  size_t start_pc = out->ops.count;
  
  struct cx_op *op = cx_op_init(out, CX_OBEGIN(), tok_idx);
  op->as_begin.child = false;
  op->as_begin.fimp = imp;

  if (imp->toks.count) {
    if (imp->args.count) {
      cx_op_init(out, CX_OPUTARGS(), tok_idx)->as_putargs.imp = imp;
    }
    
    if (!cx_compile(cx, cx_vec_start(&imp->toks), cx_vec_end(&imp->toks), out)) {
      cx_error(cx, cx->row, cx->col, "Failed compiling fimp");
      return false;
    }
  }
  
  op = cx_op_init(out,
		  CX_ORETURN(),
		  out->toks.count-1);
  op->as_return.imp = imp;
  op->as_return.pc = start_pc;  
  return true;
}

bool cx_fimp_inline(struct cx_fimp *imp,
		    size_t tok_idx,
		    struct cx_bin *out,
		    struct cx *cx) {
  if (imp->bin == out) { return true; }
  if (imp->bin) { cx_bin_deref(imp->bin); }
  imp->bin = cx_bin_ref(out);
  imp->start_pc = out->ops.count+1;
  cx_op_init(out, CX_OFIMP(), tok_idx)->as_fimp.imp = imp;
  if (!compile(imp, tok_idx, out)) { return false; }
  imp->nops = out->ops.count - imp->start_pc;
  return true;
}

bool cx_fimp_eval(struct cx_fimp *imp, struct cx_scope *scope) {
  return cx_eval(cx_test(imp->bin), imp->start_pc, scope->cx);
}

bool cx_fimp_call(struct cx_fimp *imp, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, -1);

  if (imp->ptr) {
    bool ok = imp->ptr(scope);
    cx_call_deinit(cx_vec_pop(&cx->calls));
    return ok;
  }

  return cx_fimp_eval(imp, scope);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  struct cx_fimp *imp = value->as_ptr;

  if (scope->safe && cx_fimp_score(imp, scope) == -1) {
    struct cx *cx = scope->cx;
    cx_error(cx, cx->row, cx->col, "Func not applicable: '%s'", imp->func->id);
    return -1;
  }

  return cx_fimp_call(imp, scope);
}

static struct cx_iter *iter_imp(struct cx_box *v) {
  return cx_call_iter_new(v);
}

static void write_imp(struct cx_box *value, FILE *out) {
  struct cx_fimp *imp = value->as_ptr;
  fprintf(out, "&%s<%s>", imp->func->id, imp->id);
}

static void dump_imp(struct cx_box *value, FILE *out) {
  struct cx_fimp *imp = value->as_ptr;
  fprintf(out, "Fimp(%s %s)", imp->func->id, imp->id);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  struct cx_func *fimp = v->as_ptr;
  
  fprintf(out,
	  "cx_box_init(%s, cx->fimp_type)->as_ptr = %s;\n",
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
