#include "cixl/bin.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/lambda.h"
#include "cixl/types/vect.h"
#include "cixl/op.h"
#include "cixl/scan.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

bool emit_test(struct cx *cx) {
  bool eval(struct cx *cx) {
    while (!cx->stop) {
      switch (cx->pc) {
      default:
	return true;
      }

      while (cx->scans.count) {
	struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
	if (!cx_scan_ok(s)) { break; }
	cx_vec_pop(&cx->scans);
	if (!cx_scan_call(s)) { return false; }
      }
    }

    return true;
  }

  struct cx_bin *bin = cx_bin_new();
  bin->eval = eval;
  return cx_eval(bin, 0, cx);
}

struct cx_op_type *cx_op_type_init(struct cx_op_type *type, const char *id) {
  type->id = id;
  type->eval = NULL;
  return type;
}

struct cx_op *cx_op_init(struct cx_bin *bin,
			 struct cx_op_type *type,
			 size_t tok_idx) {
  struct cx_op *op = cx_vec_push(&bin->ops);
  op->tok_idx = tok_idx;
  op->pc = bin->ops.count-1;
  op->type = type;
  return op;
}

static bool begin_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *parent = op->as_begin.child
    ? cx_scope(cx, 0)
    : op->as_begin.parent;
  
  cx_begin(cx, parent);
  cx->scan_level++;
  return true;
}

cx_op_type(CX_OBEGIN, {
    type.eval = begin_eval;
  });

static bool cut_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  cx_cut_init(cx_vec_push(&s->cuts), s);
  return true;
}

cx_op_type(CX_OCUT, {
    type.eval = cut_eval;
  });

static bool else_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_box *v = cx_pop(cx_scope(cx, 0), false);
  if (!v) { return false; }
  if (!cx_ok(v)) { cx->pc += op->as_else.nops; }
  cx_box_deinit(v);
  return true;
}

cx_op_type(CX_OELSE, {
    type.eval = else_eval;
  });

static bool end_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  if (!op->as_end.push_result) {
    struct cx_scope *s = cx_scope(cx, 0);
    cx_do_vec(&s->stack, struct cx_box, v) { cx_box_deinit(v); }
    cx_vec_clear(&s->stack);
  }
  
  cx_end(cx);
  cx->scan_level--;
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_cut *c = s->cuts.count ? cx_vec_peek(&s->cuts, 0) : NULL;
  if (c && c->scan_level == cx->scan_level) { cx_cut_deinit(cx_vec_pop(&s->cuts)); }
  return true;
}

cx_op_type(CX_OEND, {
    type.eval = end_eval;
  });

static bool fence_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  cx->scan_level += op->as_fence.delta_level;

  if (op->as_fence.delta_level < 0) {
    struct cx_scope *s = cx_scope(cx, 0);
    struct cx_cut *c = s->cuts.count ? cx_vec_peek(&s->cuts, 0) : NULL;
    if (c && c->scan_level == cx->scan_level) { cx_cut_deinit(cx_vec_pop(&s->cuts)); }
  }
  return true;
}

cx_op_type(CX_OFENCE, {
    type.eval = fence_eval;
  });

static bool on_fimp_scan(struct cx_scan *scan, void *data) {
  struct cx_op *op = data;
  struct cx_fimp *imp = op->as_fimp.imp;
  struct cx_scope *s = scan->scope;
  struct cx *cx = s->cx;
  
  if (s->safe && !cx_fimp_match(imp, s)) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: %s", imp->func->id);
    return false;
  }
  
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, cx->pc);
  cx->pc = op->pc+1;
  return true;
}

static bool fimp_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_fimp *imp = op->as_fimp.imp;
  
  if (op->as_fimp.inline1) {
    cx->pc += op->as_fimp.num_ops;
    cx_scan(cx_scope(cx, 0), imp->func, on_fimp_scan, op);
  } else {
    cx->pc += op->as_fimp.num_ops;
  }
  
  return true;
}

cx_op_type(CX_OFIMP, {
    type.eval = fimp_eval;
  });

static bool fimpdef_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_fimp *imp = op->as_fimpdef.imp;
  imp->scope = cx_scope_ref(cx_scope(cx, 0));
  return true;
}

cx_op_type(CX_OFIMPDEF, {
    type.eval = fimpdef_eval;
  });

static bool on_funcall_scan(struct cx_scan *scan, void *data) {
  struct cx_op *op = data;
  struct cx_func *func = op->as_funcall.func;
  struct cx_fimp *imp = op->as_funcall.imp;
  struct cx_scope *s = scan->scope;
  struct cx *cx = s->cx;

  if (imp) {
    if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
  } else {
    imp = op->as_funcall.jit_imp;
    if (imp && !cx_fimp_match(imp, s)) { imp = NULL; }
    if (!imp) { imp = cx_func_get_imp(func, s, 0); }
  }
  
  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
    return false;
  }
    
  op->as_funcall.jit_imp = imp;

  if (!imp->ptr) {
    struct cx_bin_func *f = cx_bin_get_func(cx->bin, imp);

    if (f) {
      cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, cx->pc);
      cx->pc = f->start_pc;
      return true;
    }
  }
  
  return cx_fimp_call(imp, s);
}

static bool funcall_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_func *func = op->as_funcall.func;
  cx_scan(cx_scope(cx, 0), func, on_funcall_scan, op);
  return true;
}

cx_op_type(CX_OFUNCALL, {
    type.eval = funcall_eval;
  });

static bool getconst_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_box *v = cx_get_const(cx, op->as_getconst.id, false);
  if (!v) { return false; }
  cx_copy(cx_push(cx_scope(cx, 0)), v);
  return true;
}

cx_op_type(CX_OGETCONST, {
    type.eval = getconst_eval;
  });

static bool getvar_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  
  if (op->as_getvar.id.id[0]) {
    struct cx_box *v = cx_get_var(s, op->as_getvar.id, false);
    if (!v) { return false; }
    cx_copy(cx_push(s), v);
  } else {
    if (!s->cuts.count) {
      cx_error(cx, tok->row, tok->col, "Nothing to uncut");
      return false;
    }

    struct cx_cut *c = cx_vec_peek(&s->cuts, 0);

    if (!c->offs) {
      cx_error(cx, tok->row, tok->col, "Nothing to uncut");
      return false;
    }

    c->offs--;

    if (c->offs < s->stack.count-1) {
      struct cx_box v = *(struct cx_box *)cx_vec_get(&s->stack, c->offs);
      cx_vec_delete(&s->stack, c->offs);
      *(struct cx_box *)cx_vec_push(&s->stack) = v;
    }
    
    if (!c->offs) { cx_cut_deinit(cx_vec_pop(&s->cuts)); }
  }
  
  return true;
}

cx_op_type(CX_OGETVAR, {
    type.eval = getvar_eval;
  });

static bool jump_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  cx->pc += op->as_jump.nops;
  return true;
}

cx_op_type(CX_OJUMP, {
    type.eval = jump_eval;
  });

static bool lambda_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *scope = cx_scope(cx, 0);
  struct cx_lambda *l = cx_lambda_new(scope,
				      op->as_lambda.start_op,
				      op->as_lambda.num_ops);
  cx_box_init(cx_push(scope), cx->lambda_type)->as_ptr = l;
  cx->pc += l->nops;
  return true;
}

cx_op_type(CX_OLAMBDA, {
    type.eval = lambda_eval;
  });

static bool push_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  cx_copy(cx_push(cx_scope(cx, 0)),  &tok->as_box);
  return true;
}

cx_op_type(CX_OPUSH, {
    type.eval = push_eval;
  });

static bool putargs_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *ds = cx_scope(cx, 0), *ss = ds->stack.count ? ds : cx_scope(cx, 1);
  struct cx_fimp *imp = op->as_putargs.imp;
  
  for (struct cx_func_arg *a = cx_vec_peek(&imp->args, 0);
       a >= (struct cx_func_arg *)imp->args.items;
       a--) {
    struct cx_box *src = cx_test(cx_pop(ss, false));

    if (a->id) {
      *cx_put_var(ds, a->sym_id, true) = *src;
    } else {
      cx_box_deinit(src);
    }
  }

  return true;
}

cx_op_type(CX_OPUTARGS, {
    type.eval = putargs_eval;
  });

static bool putvar_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_box *src = cx_pop(s, false);
  
  if (!src) { return false; }

  if (op->as_putvar.type && !cx_is(src->type, op->as_putvar.type)) {
    cx_error(cx, tok->row, tok->col,
	     "Expected type %s, actual: %s",
	     op->as_putvar.type->id, src->type->id);
    return false;
  }
  
  struct cx_box *dst = cx_put_var(s, op->as_putvar.id, true);

  if (!dst) { return false; }
  *dst = *src;
  return true;
}

cx_op_type(CX_OPUTVAR, {
    type.eval = putvar_eval;
  });

static bool on_recall_scan(struct cx_scan *scan, void *data) {
  struct cx_op *op = data;
  struct cx_fimp *imp = op->as_return.imp;
  struct cx_scope *s = scan->scope;
  struct cx *cx = s->cx;
  
  if (s->safe && !cx_fimp_match(imp, s)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    return false;
  }
  
  cx->pc = op->as_return.pc+1;
  return true;
}

static bool return_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));
  struct cx_fimp *imp = call->target;

  if (call->recalls) {
    call->recalls--;
    cx_scan(cx_scope(cx, 0), imp->func, on_recall_scan, op);
  } else {
    if (call->return_pc > -1) {
      cx->pc = call->return_pc;
    } else {
      cx->stop = true;
    }

    struct cx_scope *ss = cx_scope(cx, 0), *ds = cx_scope(cx, 1);

    if (ss->stack.count > imp->rets.count) {
      cx_error(cx, cx->row, cx->col, "Stack not empty on return");
      return false;
    }

    if (imp->rets.count) {
      if (ss->stack.count < imp->rets.count) {
	cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
	return false;
      }
      
      cx_vec_grow(&ds->stack, ds->stack.count+imp->rets.count);
      size_t i = 0;
      struct cx_func_ret *r = cx_vec_peek(&imp->rets, i);
      
      for (struct cx_box *v = cx_vec_start(&ss->stack);
	   i < ss->stack.count;
	   i++, v++, r++) {
	if (ss->safe) {
	  struct cx_type *t = r->type;
	  
	  if (!r->type) {
	    struct cx_func_arg *a = cx_vec_get(&imp->args, r->narg);
	    struct cx_box *av = cx_test(cx_get_var(ss, a->sym_id, false));
	    t = av->type;
	  }
	  
	  if (!cx_is(v->type, t)) {
	    cx_error(cx, cx->row, cx->col,
		     "Invalid return type.\nExpected %s, actual: %s",
		     t->id, v->type->id);
	    
	    return false;
	  }
	}
	
	*(struct cx_box *)cx_vec_push(&ds->stack) = *v;
      }    
    }
    
    cx_vec_clear(&ss->stack);
    cx_call_deinit(cx_vec_pop(&cx->calls));
    cx_end(cx);
    cx->scan_level--;
  }
  
  return true;
}

cx_op_type(CX_ORETURN, {
    type.eval = return_eval;
  });

static bool stash_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_vect *out = cx_vect_new();

  struct cx_cut *c = s->cuts.count ? cx_vec_peek(&s->cuts, 0) : NULL;

  if (c && c->offs) {
    for (struct cx_box *v = cx_vec_get(&s->stack, c->offs);
	 v != cx_vec_end(&s->stack);
	 v++) {
      *(struct cx_box *)cx_vec_push(&out->imp) = *v;
    }

    s->stack.count = c->offs;
  } else {
    out->imp = s->stack;
    cx_vec_init(&s->stack, sizeof(struct cx_box));
  }
  
  cx_box_init(cx_push(s), s->cx->vect_type)->as_ptr = out;
  return true;
}

cx_op_type(CX_OSTASH, {
    type.eval = stash_eval;
  });

static bool stop_eval(struct cx_op *op, struct cx_tok *tok, struct cx *cx) {
  cx->stop = true;
  return true;
}

cx_op_type(CX_OSTOP, {
    type.eval = stop_eval;
  });
