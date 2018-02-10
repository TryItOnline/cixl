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

bool cx_emit_tests(struct cx *cx) {
  bool eval(struct cx *cx) {
    while (!cx->stop) {
      switch (cx->pc) {

      case 0: {
	cx->row = 1; cx->col = 0;
	struct cx_func *func = cx_get_func(cx, "say", false);
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0), func, cx_fimp_scan);
	scan->as_fimp.imp = cx_func_get_imp(func, "A", false);
	scan->as_fimp.pc = 1;
	cx->pc += 10;
	break;
      }
      case 1: {
	cx->row = 1; cx->col = 0;
	struct cx_scope *parent = NULL;
	struct cx_func *func = cx_get_func(cx, "say", false);
	struct cx_fimp *imp = cx_func_get_imp(func, "A", false);
	parent = imp->scope;
	cx_begin(cx, parent);
	cx->scan_level++;
	cx->pc++;
      }
      case 2: {
	cx->row = 1; cx->col = 0;
	struct cx_func *func = cx_get_func(cx, "say", false);
	struct cx_fimp *imp = cx_func_get_imp(func, "A", false);
	cx_oputargs(imp, cx);
	cx->pc++;
      }
      case 3: {
	cx->row = 1; cx->col = 0;
	struct cx_box *v = cx_get_const(cx, cx_sym(cx, "out"), false);
	if (!v) { return false; }
	cx_copy(cx_push(cx_scope(cx, 0)), v);
	cx->pc++;
	break;
      }
      case 4: {
	cx->row = 1; cx->col = 5;
	struct cx_func *func = cx_get_func(cx, "print", false);
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0), func, cx_funcall_scan);
	scan->as_funcall.imp = NULL;
	cx->pc++;
	break;
      }
      case 5: {
	cx->row = 1; cx->col = 11;
	cx_ogetvar1(cx_sym(cx, "v"), cx_scope(cx, 0));
	cx->pc++;
	break;
      }
      case 6: {
	cx->row = 1; cx->col = 14;
	struct cx_box *v = cx_get_const(cx, cx_sym(cx, "out"), false);
	if (!v) { return false; }
	cx_copy(cx_push(cx_scope(cx, 0)), v);
	cx->pc++;
	break;
      }
      case 7: {
	cx->row = 1; cx->col = 19;
	struct cx_func *func = cx_get_func(cx, "print", false);
	struct cx_scan *scan = cx_scan(cx_scope(cx, 0), func, cx_funcall_scan);
	scan->as_funcall.imp = NULL;
	cx->pc++;
	break;
      }
      case 8: {
	cx->row = 1; cx->col = 25;
	cx_box_init(cx_push(cx_scope(cx, 0)), cx->char_type)->as_char = 10;
	cx->pc++;
	break;
      }
      case 9: {
	cx->row = 1; cx->col = 25;
	struct cx_func *func = cx_get_func(cx, "say", false);
	struct cx_fimp *imp = cx_func_get_imp(func, "A", false);
	cx_oreturn(imp, 1);
	cx->pc++;
	break;
      }
      case 10: {
	cx->row = 1; cx->col = 6;
	cx_box_init(cx_push(cx_scope(cx, 0)), cx->int_type)->as_int = 42;
	cx->pc++;
	break;
      }
  
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
  bool ok = cx_eval(bin, 0, cx);
  cx_bin_deref(bin);
  return ok;
}

static bool emit(struct cx_op *op, struct cx_bin *bin, FILE *out, struct cx *cx) {
  cx_error(cx, op->row, op->col, "Emit not implemented: %s", op->type->id);
  return false;
}

struct cx_op_type *cx_op_type_init(struct cx_op_type *type, const char *id) {
  type->id = id;
  type->init = NULL;
  type->deinit = NULL;
  type->eval = NULL;
  type->emit = emit;
  return type;
}

struct cx_op *cx_op_init(struct cx_bin *bin,
			 struct cx_op_type *type,
			 size_t tok_idx) {
  struct cx_op *op = cx_vec_push(&bin->ops);
  op->type = type;
  op->tok_idx = tok_idx;
  op->pc = bin->ops.count-1;
  op->row = -1; op->col = -1;
  return op;
}

static bool begin_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *parent = op->as_begin.child
    ? cx_scope(cx, 0)
    : op->as_begin.fimp->scope;
  
  cx_begin(cx, parent);
  cx->scan_level++;
  return true;
}

static bool begin_emit(struct cx_op *op,
		       struct cx_bin *bin,
		       FILE *out,
		       struct cx *cx) {
  fputs("struct cx_scope *parent = NULL;\n", out);
  
  if (op->as_begin.child) {
    fputs("parent = cx_scope(cx, 0);\n", out);
  } else {
    struct cx_fimp *imp = op->as_begin.fimp;
    
    fprintf(out,
	    "struct cx_func *func = cx_get_func(cx, \"%s\", false);\n",
	    imp->func->id);
    
    fprintf(out, "struct cx_fimp *imp = cx_func_get_imp(func, \"%s\", false);\n",
	    imp->id);
    fputs("parent = imp->scope;\n", out);
  }

  fputs("cx_begin(cx, parent);\n", out);
  fputs("cx->scan_level++;\n", out);
  fputs("cx->pc++;\n", out);
  return true;
}

cx_op_type(CX_OBEGIN, {
    type.eval = begin_eval;
    type.emit = begin_emit;
  });

static bool cut_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  cx_cut_init(cx_vec_push(&s->cuts), s);
  return true;
}

cx_op_type(CX_OCUT, {
    type.eval = cut_eval;
  });

static bool else_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_box *v = cx_pop(cx_scope(cx, 0), false);
  if (!v) { return false; }
  if (!cx_ok(v)) { cx->pc += op->as_else.nops; }
  cx_box_deinit(v);
  return true;
}

cx_op_type(CX_OELSE, {
    type.eval = else_eval;
  });

static bool end_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
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

static bool fence_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx->scan_level += op->as_fence.delta_level;

  if (op->as_fence.delta_level < 0) {
    struct cx_scope *s = cx_scope(cx, 0);
    struct cx_cut *c = s->cuts.count ? cx_vec_peek(&s->cuts, 0) : NULL;
    if (c && c->scan_level == cx->scan_level) { cx_cut_deinit(cx_vec_pop(&s->cuts)); }
  }
  return true;
}

static bool fence_emit(struct cx_op *op,
		       struct cx_bin *bin,
		       FILE *out,
		       struct cx *cx) {
  fprintf(out, "cx->scan_level += %d;\n", op->as_fence.delta_level);

  if (op->as_fence.delta_level < 0) {
    fputs("struct cx_scope *s = cx_scope(cx, 0);\n"
	  "struct cx_cut *c = s->cuts.count ? cx_vec_peek(&s->cuts, 0) : NULL;\n"
	  "if (c && c->scan_level == cx->scan_level) { "
	  "cx_cut_deinit(cx_vec_pop(&s->cuts)); "
	  "}", out);
  }

  fputs("cx->pc++;\n", out);
  fputs("break;\n", out);
  return true;
}

cx_op_type(CX_OFENCE, {
    type.eval = fence_eval;
    type.emit = fence_emit;
  });

bool cx_fimp_scan(struct cx_scan *scan) {
  struct cx_fimp *imp = scan->as_fimp.imp;
  struct cx_scope *s = scan->scope;
  struct cx *cx = s->cx;
  
  if (s->safe && !cx_fimp_match(imp, s)) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: %s", imp->func->id);
    return false;
  }
  
  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, cx->pc);
  cx->pc = scan->as_fimp.pc;
  return true;
}

static bool fimp_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_fimp *imp = op->as_fimp.imp;
  
  if (op->as_fimp.inline1) {
    struct cx_scan *scan = cx_scan(cx_scope(cx, 0), imp->func, cx_fimp_scan);
    scan->as_fimp.imp = imp;
    scan->as_fimp.pc = op->pc+1;
  }
  
  cx->pc += op->as_fimp.nops;
  return true;
}

static bool fimp_emit(struct cx_op *op,
		      struct cx_bin *bin,
		      FILE *out,
		      struct cx *cx) {
  if (op->as_fimp.inline1) {
    struct cx_fimp *imp = op->as_fimp.imp;
    fprintf(out, "struct cx_func *func = cx_get_func(cx, \"%s\", false);\n",
	    imp->func->id);
    fputs("struct cx_scan *scan = cx_scan(cx_scope(cx, 0), func, cx_fimp_scan);\n",
	  out);
    fprintf(out, "scan->as_fimp.imp = cx_func_get_imp(func, \"%s\", false);\n",
	    imp->id);
    fprintf(out, "scan->as_fimp.pc = %zd;\n", op->pc+1);
  }

  fprintf(out, "cx->pc += %zd;\n", op->as_fimp.nops+1);
  fputs("break;\n", out);
  return true;
}

cx_op_type(CX_OFIMP, {
    type.eval = fimp_eval;
    type.emit = fimp_emit;
  });

static bool fimpdef_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_fimp *imp = op->as_fimpdef.imp;
  imp->scope = cx_scope_ref(cx_scope(cx, 0));
  return true;
}

cx_op_type(CX_OFIMPDEF, {
    type.eval = fimpdef_eval;
  });

bool cx_funcall_scan(struct cx_scan *scan) {
  struct cx_func *func = scan->func;
  struct cx_fimp *imp = scan->as_funcall.imp;
  struct cx_scope *s = scan->scope;
  struct cx *cx = s->cx;

  if (imp) {
    if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
  } else {
    imp = cx_func_match_imp(func, s, 0);
  }
  
  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
    return false;
  }
    
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

static bool funcall_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_func *func = op->as_funcall.func;
  struct cx_scan *scan = cx_scan(cx_scope(cx, 0), func, cx_funcall_scan);
  scan->as_funcall.imp = op->as_funcall.imp;
  return true;
}

static bool cx_funcall_emit(struct cx_op *op,
			    struct cx_bin *bin,
			    FILE *out,
			    struct cx *cx) {
  struct cx_func *func = op->as_funcall.func;
  fprintf(out, "struct cx_func *func = cx_get_func(cx, \"%s\", false);\n", func->id);
  fputs("struct cx_scan *scan = cx_scan(cx_scope(cx, 0), func, cx_funcall_scan);\n",
	out);

  struct cx_fimp *imp = op->as_funcall.imp;

  if (imp) {
    fprintf(out, "scan->as_funcall.imp = cx_func_get_imp(func, \"%s\", false);\n",
	    imp->id);
  } else {
    fputs("scan->as_funcall.imp = NULL;\n", out);
  }
  
  fputs("cx->pc++;\n", out);
  fputs("break;\n", out);
  return true;
}

cx_op_type(CX_OFUNCALL, {
    type.eval = funcall_eval;
    type.emit = cx_funcall_emit;
  });

static bool getconst_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_box *v = cx_get_const(cx, op->as_getconst.id, false);
  if (!v) { return false; }
  cx_copy(cx_push(cx_scope(cx, 0)), v);
  return true;
}

static bool getconst_emit(struct cx_op *op,
			  struct cx_bin *bin,
			  FILE *out,
			  struct cx *cx) {
  fprintf(out, "struct cx_box *v = cx_get_const(cx, cx_sym(cx, \"%s\"), false);\n",
	  op->as_getconst.id.id);
  fputs("if (!v) { return false; }\n", out);
  fputs("cx_copy(cx_push(cx_scope(cx, 0)), v);\n", out);
  fputs("cx->pc++;\n", out);
  fputs("break;\n", out);
  return true;
}

cx_op_type(CX_OGETCONST, {
    type.eval = getconst_eval;
    type.emit = getconst_emit;
  });


bool cx_ogetvar1(struct cx_sym id, struct cx_scope *scope) {
  struct cx_box *v = cx_get_var(scope, id, false);
  if (!v) { return false; }
  cx_copy(cx_push(scope), v);
  return true;
}

bool cx_ogetvar2(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  if (!scope->cuts.count) {
    cx_error(cx, cx->row, cx->col, "Nothing to uncut");
    return false;
  }
  
  struct cx_cut *c = cx_vec_peek(&scope->cuts, 0);
  
  if (!c->offs) {
    cx_error(cx, cx->row, cx->col, "Nothing to uncut");
    return false;
  }
  
  c->offs--;
  
  if (c->offs < scope->stack.count-1) {
    struct cx_box v = *(struct cx_box *)cx_vec_get(&scope->stack, c->offs);
    cx_vec_delete(&scope->stack, c->offs);
    *(struct cx_box *)cx_vec_push(&scope->stack) = v;
  }
  
  if (!c->offs) { cx_cut_deinit(cx_vec_pop(&scope->cuts)); }
  return true;
}

static bool getvar_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_sym id = op->as_getvar.id;
  return id.id[0] ? cx_ogetvar1(id, s) : cx_ogetvar2(s);
}

static bool getvar_emit(struct cx_op *op,
			struct cx_bin *bin,
			FILE *out,
			struct cx *cx) {
  struct cx_sym id = op->as_getvar.id;

  if (id.id[0]) {
    fprintf(out, "cx_ogetvar1(cx_sym(cx, \"%s\"), cx_scope(cx, 0));\n", id.id);
  } else {
    fputs("cx_ogetvar2(cx_scope(cx, 0));\n", out);
  }

  fputs("cx->pc++;\n", out);
  fputs("break;\n", out);  
  return true;
}


cx_op_type(CX_OGETVAR, {
    type.eval = getvar_eval;
    type.emit = getvar_emit;
  });

static bool jump_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx->pc += op->as_jump.nops;
  return true;
}

cx_op_type(CX_OJUMP, {
    type.eval = jump_eval;
  });

static bool lambda_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *scope = cx_scope(cx, 0);
  struct cx_lambda *l = cx_lambda_new(scope,
				      op->as_lambda.start_op,
				      op->as_lambda.nops);
  cx_box_init(cx_push(scope), cx->lambda_type)->as_ptr = l;
  cx->pc += l->nops;
  return true;
}

static bool lambda_emit(struct cx_op *op,
			struct cx_bin *bin,
			FILE *out,
			struct cx *cx) {
  fputs("struct cx_scope *s = cx_scope(cx, 0);\n", out);
  fprintf(out, "struct cx_lambda *l = cx_lambda_new(s, %zd, %zd);\n",
	  op->as_lambda.start_op, op->as_lambda.nops);
  fputs("cx_box_init(cx_push(s), cx->lambda_type)->as_ptr = l;\n", out);
  fputs("cx->pc += l->nops+1;\n", out);
  fputs("break;\n", out);  
  return true;
}

cx_op_type(CX_OLAMBDA, {
    type.eval = lambda_eval;
    type.emit = lambda_emit;
  });

static void push_init(struct cx_op *op, struct cx_tok *tok) {
  cx_copy(&op->as_push.value, &tok->as_box);
}

static void push_deinit(struct cx_op *op) {
  cx_box_deinit(&op->as_push.value);
}

static bool push_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx_copy(cx_push(cx_scope(cx, 0)),  &op->as_push.value);
  return true;
}

static bool push_emit(struct cx_op *op,
		      struct cx_bin *bin,
		      FILE *out,
		      struct cx *cx) {
  cx_box_emit(&op->as_push.value, out);
  fputs("cx->pc++;\n", out);
  fputs("break;\n", out);  
  return true;
}

cx_op_type(CX_OPUSH, {
    type.init = push_init;
    type.deinit = push_deinit;
    type.eval = push_eval;
    type.emit = push_emit;
  });

void cx_oputargs(struct cx_fimp *imp, struct cx *cx) {
  struct cx_scope *ds = cx_scope(cx, 0), *ss = ds->stack.count ? ds : cx_scope(cx, 1);

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
}

static bool putargs_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx_oputargs(op->as_putargs.imp, cx);
  return true;
}

static bool putargs_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  struct cx_fimp *imp = op->as_putargs.imp;
  
  fprintf(out,
	  "struct cx_func *func = cx_get_func(cx, \"%s\", false);\n",
	  imp->func->id);
  fprintf(out, "struct cx_fimp *imp = cx_func_get_imp(func, \"%s\", false);\n",
	  imp->id);
  fputs("cx_oputargs(imp, cx);\n", out);
  fputs("cx->pc++;\n", out);
  return true;
}

cx_op_type(CX_OPUTARGS, {
    type.eval = putargs_eval;
    type.emit = putargs_emit;
  });

static bool putvar_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_box *src = cx_pop(s, false);
  
  if (!src) { return false; }

  if (op->as_putvar.type && !cx_is(src->type, op->as_putvar.type)) {
    cx_error(cx, op->row, op->col,
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

bool cx_recall_scan(struct cx_scan *scan) {
  struct cx_fimp *imp = scan->as_recall.imp;
  struct cx_scope *s = scan->scope;
  struct cx *cx = s->cx;
  
  if (s->safe && !cx_fimp_match(imp, s)) {
    cx_error(cx, cx->row, cx->col, "Recall not applicable");
    return false;
  }
  
  cx->pc = scan->as_recall.pc+1;
  return true;
}

bool cx_oreturn(struct cx_fimp *imp, size_t pc) {
  struct cx *cx = imp->func->cx;
  struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));

  if (call->recalls) {
    call->recalls--;
    struct cx_scan *scan = cx_scan(cx_scope(cx, 0), imp->func, cx_recall_scan);
    scan->as_recall.imp = imp;
    scan->as_recall.pc = pc;
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

static bool return_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  return cx_oreturn(op->as_return.imp, op->as_return.pc);
}

static bool return_emit(struct cx_op *op,
			struct cx_bin *bin,
			FILE *out,
			struct cx *cx) {
  struct cx_fimp *imp = op->as_return.imp;
  struct cx_func *func = imp->func;
  
  fprintf(out, "struct cx_func *func = cx_get_func(cx, \"%s\", false);\n", func->id);
  fprintf(out, "struct cx_fimp *imp = cx_func_get_imp(func, \"%s\", false);\n",
	  imp->id);

  fprintf(out, "cx_oreturn(imp, %zd);\n", op->as_return.pc);
  fputs("cx->pc++;\n", out);
  fputs("break;\n", out);
  return true;
}

cx_op_type(CX_ORETURN, {
    type.eval = return_eval;
    type.emit = return_emit;
  });

static bool stash_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
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

static bool stop_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx->stop = true;
  return true;
}

cx_op_type(CX_OSTOP, {
    type.eval = stop_eval;
  });
