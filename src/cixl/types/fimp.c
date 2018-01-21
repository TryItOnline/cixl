#include <inttypes.h>

#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/call_iter.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/tok.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

struct cx_fimp *cx_fimp_init(struct cx_fimp *imp,
			     struct cx_func *func,
			     char *id,
			     size_t i) {
  imp->func = func;
  imp->id = id;
  imp->i = i;
  imp->ptr = NULL;
  imp->scope = NULL;
  imp->bin = NULL;
  cx_vec_init(&imp->args, sizeof(struct cx_func_arg));
  cx_vec_init(&imp->rets, sizeof(struct cx_func_ret));
  cx_vec_init(&imp->toks, sizeof(struct cx_tok));
  return imp;
}

struct cx_fimp *cx_fimp_deinit(struct cx_fimp *imp) {
  free(imp->id);
       
  cx_do_vec(&imp->args, struct cx_func_arg, a) { cx_func_arg_deinit(a); }
  cx_vec_deinit(&imp->args);

  cx_vec_deinit(&imp->rets);
  
  cx_do_vec(&imp->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&imp->toks);

  if (imp->scope) { cx_scope_deref(imp->scope); }
  if (imp->bin) { cx_bin_deref(imp->bin); }
  return imp;
}

bool cx_fimp_match(struct cx_fimp *imp, struct cx_vec *stack) {
  if (stack->count < imp->args.count) { return false; }
  if (!imp->args.count) { return true; }
  
  struct cx_func_arg *i = (struct cx_func_arg *)cx_vec_end(&imp->args)-1;
  struct cx_box *j = (struct cx_box *)cx_vec_end(stack)-1;
  
  for (; i >= (struct cx_func_arg *)imp->args.items &&
	 j >= (struct cx_box *)stack->items;
       i--, j--) {    
    struct cx_type *t = i->type;

    if (!t && i->narg != -1) {
      t = ((struct cx_box *)cx_vec_get(stack,
				       stack->count-imp->args.count+i->narg))->type;
    }

    if (t) {
      if (!cx_is(j->type, t)) { return false; }
    } else if (!cx_eqval(&i->value, j)) {
      return false;
    }
  }

  return true;
}

bool cx_fimp_compile(struct cx_fimp *imp,
		     size_t tok_idx,
		     bool inline1,
		     struct cx_bin *out) {
  struct cx *cx = imp->func->cx;
  size_t start_op = out->ops.count;
  
  struct cx_op *op = cx_op_init(cx_vec_push(&out->ops),
				CX_OBEGIN(),
				tok_idx);
  op->as_begin.child = false;
  op->as_begin.parent = imp->scope;

  if (imp->toks.count) {
    if (imp->args.count) {
      cx_op_init(cx_vec_push(&out->ops),
		 CX_OPUTARGS(),
		 tok_idx)->as_putargs.imp = imp;
    }
    
    if (!cx_compile(cx, cx_vec_start(&imp->toks), cx_vec_end(&imp->toks), out)) {
      cx_error(cx, cx->row, cx->col, "Failed compiling fimp");
      return false;
    }
  }
  
  cx_op_init(cx_vec_push(&out->ops),
	     CX_ORETURN(),
	     out->toks.count-1)->as_return.start_op = start_op;
  
  if (!inline1) { cx_bin_add_func(out, imp, start_op); }
  return true;
}

bool cx_fimp_eval(struct cx_fimp *imp, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_bin_func *bin = cx_bin_get_func(cx->bin, imp);
  bool ok = false;

  if (bin) {
    ok = cx_eval(cx, cx->bin, cx_vec_get(&cx->bin->ops, bin->start_op));
  } else {
    if (!imp->bin) {
      imp->bin = cx_bin_new();
      if (!cx_fimp_compile(imp, 0, false, imp->bin)) { return false; }
    }
    
    ok = cx_eval(cx, imp->bin, NULL);
  }
  
  return ok;
}

bool cx_fimp_call(struct cx_fimp *imp, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, NULL);

  if (imp->ptr) {
    imp->ptr(scope);
    cx_call_deinit(cx_vec_pop(&cx->calls));
    return true;
  }

  bool ok = cx_fimp_eval(imp, scope);
  return ok;
}

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

struct cx_type *cx_init_fimp_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Fimp", cx->seq_type);
  t->equid = equid_imp;
  t->call = call_imp;
  t->iter = iter_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  return t;
}
