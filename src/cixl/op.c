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
#include "cixl/scope.h"
#include "cixl/tok.h"

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
  type->emit_func = NULL;
  type->emit_fimp = NULL;
  type->emit_syms = NULL;
  type->emit_types = NULL;
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
  return true;
}

static bool begin_emit(struct cx_op *op,
		       struct cx_bin *bin,
		       FILE *out,
		       struct cx *cx) {
  fputs("struct cx_scope *parent = ", out);
  
  if (op->as_begin.child) {
    fputs("cx_scope(cx, 0);\n", out);
  } else {
    struct cx_fimp *imp = op->as_begin.fimp;
    fprintf(out, "fimp%zd_%zd->scope;\n", imp->func->tag, imp->idx);
  }

  fputs("cx_begin(cx, parent);\n", out);
  return true;
}

static struct cx_func *begin_emit_func(struct cx_op *op) {
  return op->as_begin.fimp ? op->as_begin.fimp->func : NULL;
}

static struct cx_fimp *begin_emit_fimp(struct cx_op *op) {
  return op->as_begin.fimp;
}

cx_op_type(CX_OBEGIN, {
    type.eval = begin_eval;
    type.emit = begin_emit;
    type.emit_func = begin_emit_func;
    type.emit_fimp = begin_emit_fimp;
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
  cx_end(cx);
  return true;
}

static bool end_emit(struct cx_op *op, struct cx_bin *bin, FILE *out, struct cx *cx) {
  fputs("cx_end(cx);\n", out);
  return true;
}

cx_op_type(CX_OEND, {
    type.eval = end_eval;
    type.emit = end_emit;
  });

static bool fimp_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_fimp *imp = op->as_fimp.imp;
  
  if (op->as_fimp.inline1) {
    struct cx_scope *s = cx_scope(cx, 0);
    
    if (s->safe && !cx_fimp_match(imp, s)) {
      cx_error(cx, cx->row, cx->col, "Func not applicable: %s", imp->func->id);
      return false;
    }
    
    cx_call_init(cx_vec_push(&cx->calls),
		 cx->row, cx->col,
		 imp,
		 cx->pc+op->as_fimp.nops);
  } else {
    cx->pc += op->as_fimp.nops;
  }
  
  return true;
}

static bool fimp_emit(struct cx_op *op,
		      struct cx_bin *bin,
		      FILE *out,
		      struct cx *cx) {
  struct cx_fimp *imp = op->as_fimp.imp;

  if (op->as_fimp.inline1) {
    fprintf(out,
	    "struct cx_scope *s = cx_scope(cx, 0);\n"
	    "struct cx_fimp *imp = fimp%zd_%zd;\n"
	    "if (s->safe && !cx_fimp_match(imp, s)) {\n"
	    "  cx_error(cx, cx->row, cx->col, \"Func not applicable: %%s\",\n"
	    " 	        imp->func->id);\n"
	    "  return false;\n"
	    "}\n"
            "cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, %zd);\n",
	    imp->func->tag, imp->idx, op->pc+op->as_fimp.nops+1);
  } else {
    size_t pc = op->pc+op->as_fimp.nops+1;
    
    fprintf(out,
	    "cx->pc = %zd;\n"
	    "goto op%zd;\n",
	    pc, pc);
  }
  
  return true;
}

static struct cx_func *fimp_emit_func(struct cx_op *op) {
  return op->as_fimp.imp->func;
}

static struct cx_fimp *fimp_emit_fimp(struct cx_op *op) {
  return op->as_fimp.imp;
}

cx_op_type(CX_OFIMP, {
    type.eval = fimp_eval;
    type.emit = fimp_emit;
    type.emit_func = fimp_emit_func;
    type.emit_fimp = fimp_emit_fimp;
  });

static bool fimpdef_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_fimp *imp = op->as_fimpdef.imp;
  imp->scope = cx_scope_ref(cx_scope(cx, 0));
  return true;
}

cx_op_type(CX_OFIMPDEF, {
    type.eval = fimpdef_eval;
  });

static bool funcall_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_func *func = op->as_funcall.func;
  struct cx_fimp *imp = op->as_funcall.imp;
  struct cx_scope *s = cx_scope(cx, 0);
  
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

static bool funcall_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  struct cx_func *func = op->as_funcall.func;
  struct cx_fimp *imp = op->as_funcall.imp;

  fputs("struct cx_scope *s = cx_scope(cx, 0);\n", out);
  fprintf(out, "struct cx_func *func = func%zd;\n", func->tag);
  fputs("struct cx_fimp *imp = ", out);
  
  if (imp) {
    fprintf(out,
	    "fimp%zd_%zd;\n"
	    "if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }\n",
	    func->tag, imp->idx);
  } else {
    fputs("cx_func_match_imp(func, s, 0);\n", out);
  }
  
  fputs("if (!imp) {\n"
	"  cx_error(cx, cx->row, cx->col, \"Func not applicable: %%s\", func->id);\n"
	"  return false;\n"
	"}\n",
	out);
    
  if (imp && !imp->ptr) {
    struct cx_bin_func *f = cx_test(cx_bin_get_func(cx->bin, imp));

    fprintf(out,
	    "cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, cx->pc);\n"
	    "cx->pc = %zd;\n"
	    "goto op%zd;\n",
	    f->start_pc, f->start_pc);
  } else {
    fputs("if (!cx_fimp_call(imp, s)) { return false; }\n", out);
  }
  
  return true;
}

static struct cx_func *funcall_emit_func(struct cx_op *op) {
  return op->as_funcall.func;
}

static struct cx_fimp *funcall_emit_fimp(struct cx_op *op) {
  return op->as_funcall.imp;
}

cx_op_type(CX_OFUNCALL, {
    type.eval = funcall_eval;
    type.emit = funcall_emit;
    type.emit_func = funcall_emit_func;
    type.emit_fimp = funcall_emit_fimp;
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
  fprintf(out, "struct cx_box *v = cx_get_const(cx, sym%zd, false);\n",
	  op->as_getconst.id.tag);

  fputs("if (!v) { return false; }\n"
	"cx_copy(cx_push(cx_scope(cx, 0)), v);\n",
	out);
  
  return true;
}

static void getconst_emit_syms(struct cx_op *op, struct cx_vec *out) {
  *(struct cx_sym *)cx_vec_push(out) = op->as_getconst.id;
}

cx_op_type(CX_OGETCONST, {
    type.eval = getconst_eval;
    type.emit = getconst_emit;
    type.emit_syms = getconst_emit_syms;
  });

bool cx_ogetvar(struct cx_sym id, struct cx_scope *scope) {
  struct cx_box *v = cx_get_var(scope, id, false);
  if (!v) { return false; }
  cx_copy(cx_push(scope), v);
  return true;
}

static bool getvar_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_sym id = op->as_getvar.id;
  return cx_ogetvar(id, s);
}

static bool getvar_emit(struct cx_op *op,
			struct cx_bin *bin,
			FILE *out,
			struct cx *cx) {
  struct cx_sym id = op->as_getvar.id;
  fprintf(out, "cx_ogetvar(sym%zd, cx_scope(cx, 0));\n", id.tag);
  return true;
}

static void getvar_emit_syms(struct cx_op *op, struct cx_vec *out) {
  struct cx_sym id = op->as_getvar.id;
  if (id.id[0]) { *(struct cx_sym *)cx_vec_push(out) = id; }
}

cx_op_type(CX_OGETVAR, {
    type.eval = getvar_eval;
    type.emit = getvar_emit;
    type.emit_syms = getvar_emit_syms;
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

  size_t pc = op->pc+op->as_lambda.nops+1;
  
  fprintf(out,
	  "cx->pc = %zd;\n"
	  "goto op%zd;\n",
	  pc, pc);

  return true;
}

cx_op_type(CX_OLAMBDA, {
    type.eval = lambda_eval;
    type.emit = lambda_emit;
  });

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
  return cx_box_emit(&op->as_push.value, out);
}

cx_op_type(CX_OPUSH, {
    type.deinit = push_deinit;
    type.eval = push_eval;
    type.emit = push_emit;
  });

static bool putargs_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_fimp *imp = op->as_putargs.imp;
  struct cx_scope *ds = cx_scope(cx, 0), *ss = ds->stack.count ? ds : cx_scope(cx, 1);

  for (struct cx_func_arg *a = cx_vec_peek(&imp->args, 0);
       a >= (struct cx_func_arg *)imp->args.items;
       a--) {
    struct cx_box *src = cx_pop(ss, false);
    if (!src) { return false; }
    
    if (a->id) {
      *cx_put_var(ds, a->sym_id, true) = *src;
    } else {
      cx_box_deinit(src);
    }
  }

  return true;
}

static bool putargs_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  struct cx_fimp *imp = op->as_putargs.imp;

  fputs("struct cx_scope\n"
	"*ds = cx_scope(cx, 0),\n"
	"*ss = ds->stack.count ? ds : cx_scope(cx, 1);\n",
	out);

  for (struct cx_func_arg *a = cx_vec_peek(&imp->args, 0);
       a >= (struct cx_func_arg *)imp->args.items;
       a--) {
    if (a->id) {
      fprintf(out,
	      "  *cx_put_var(ds, sym%zd, true) = *cx_test(cx_pop(ss, false));\n",
	      a->sym_id.tag);
    } else {
      fputs("  cx_box_deinit(cx_test(cx_pop(ss, false)));\n", out);
    }
  }
  
  return true;
}

static struct cx_func *putargs_emit_func(struct cx_op *op) {
  return op->as_putargs.imp->func;
}

static struct cx_fimp *putargs_emit_fimp(struct cx_op *op) {
  return op->as_putargs.imp;
}

static void putargs_emit_syms(struct cx_op *op, struct cx_vec *out) {
  struct cx_fimp *imp = op->as_putargs.imp;

  for (struct cx_func_arg *a = cx_vec_peek(&imp->args, 0);
       a >= (struct cx_func_arg *)imp->args.items;
       a--) {
    *(struct cx_sym *)cx_vec_push(out) = a->sym_id;
  }
}

cx_op_type(CX_OPUTARGS, {
    type.eval = putargs_eval;
    type.emit = putargs_emit;
    type.emit_func = putargs_emit_func;
    type.emit_fimp = putargs_emit_fimp;
    type.emit_syms = putargs_emit_syms;
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

void cx_oreturn_end(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  cx_vec_clear(&scope->stack);

  struct cx_call *call = cx_vec_pop(&cx->calls);
  
  if (call->return_pc > -1) {
    cx->pc = call->return_pc;
  } else {
    cx->stop = true;
  }

  cx_call_deinit(call);
  cx_end(cx);
}

static bool return_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_fimp *imp = op->as_return.imp;
  struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));

  if (call->recalls) {
    call->recalls--;
    struct cx_scope *s = cx_scope(cx, 0);
    
    if (s->safe && !cx_fimp_match(imp, s)) {
      cx_error(cx, cx->row, cx->col, "Recall not applicable");
      return false;
    }
    
    cx->pc = op->as_return.pc+1;
  } else {
    struct cx_scope *ss = cx_scope(cx, 0);

    if (ss->stack.count > imp->rets.count) {
      cx_error(cx, cx->row, cx->col, "Stack not empty on return");
      return false;
    }
    
    if (ss->stack.count < imp->rets.count) {
      cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
      return false;
    }

    if (imp->rets.count) {
      struct cx_scope *ds = cx_scope(cx, 1);
      cx_vec_grow(&ds->stack, ds->stack.count+imp->rets.count);
      size_t i = 0;
      struct cx_func_ret *r = cx_vec_start(&imp->rets);
      
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

    cx_oreturn_end(ss);
  }
  
  return true;
}

static bool return_emit(struct cx_op *op,
			struct cx_bin *bin,
			FILE *out,
			struct cx *cx) {
  struct cx_fimp *imp = op->as_return.imp;

  fputs("struct cx_call *call = cx_test(cx_vec_peek(&cx->calls, 0));\n"
	"struct cx_scope *s = cx_scope(cx, 0);\n\n"
	"if (call->recalls) {\n",
	out);

  size_t pc = op->as_return.pc+1;
  
  fprintf(out,
	  "  if (s->safe && !cx_fimp_match(fimp%zd_%zd, s)) {\n"
	  "    cx_error(cx, cx->row, cx->col, \"Recall not applicable\");\n"
	  "    return false;\n"
	  "  }\n\n"
	  "  call->recalls--;\n"
	  "  cx->pc = %zd;\n"
	  "  goto op%zd;\n",
          imp->func->tag, imp->idx, pc, pc);

  fprintf(out,
	  "} else {\n"
	  "  if (s->stack.count > %zd) {\n"
	  "    cx_error(cx, cx->row, cx->col, \"Stack not empty on return\");\n"
	  "    return false;\n"
	  "  }\n\n"
	  "  if (s->stack.count < %zd) {\n"
	  "    cx_error(cx, cx->row, cx->col,\n"
	  "             \"Not enough return values on stack\");\n"
	  "    return false;\n"
	  "  }\n\n",
	  imp->rets.count, imp->rets.count);
  
  if (imp->rets.count) {
    fputs("struct cx_scope *ds = cx_scope(cx, 1);\n", out);
    fprintf(out, "cx_vec_grow(&ds->stack, ds->stack.count+%zd);\n", imp->rets.count);
    fputs("struct cx_box *v = cx_vec_start(&s->stack);\n\n", out);
    
    for (struct cx_func_ret *r = cx_vec_start(&imp->rets);
	 r != cx_vec_end(&imp->rets);
	 r++) {
      fputs("  if (s->safe) {\n"
	    "    struct cx_type *t = NULL;\n",
	    out);
      
      if (r->type) {
	fprintf(out, "    t = type%zd;\n", r->type->tag);
      } else {
	fprintf(out,
		"    struct cx_func_arg *a = cx_vec_get(&fimp%zd_%zd->args, %d);\n"
		"    struct cx_box *av = cx_test(cx_get_var(s, a->sym_id, false));\n"
		"    t = av->type;\n",
		imp->func->tag, imp->idx, r->narg);
      }
      
      fputs("    if (!cx_is(v->type, t)) {\n"
	    "      cx_error(cx, cx->row, cx->col,\n"
	    "               \"Invalid return type.\\n\"\n"
            "               \"Expected %s, actual: %s\",\n"
	    "               t->id, v->type->id);\n"
	    "      return false;\n"
	    "    }\n"
	    "  }\n\n"
	    "  *(struct cx_box *)cx_vec_push(&ds->stack) = *v;\n"
	    "  v++;\n\n",
	    out);
    }
  } 
  
  fputs("  cx_oreturn_end(s);\n"
	"}\n",
	out);
  
  return true;  
}

static struct cx_func *return_emit_func(struct cx_op *op) {
  return op->as_return.imp->func;
}

static struct cx_fimp *return_emit_fimp(struct cx_op *op) {
  return op->as_return.imp;
}

static void return_emit_types(struct cx_op *op, struct cx_vec *out) {
  struct cx_fimp *imp = op->as_return.imp;
  
  for (struct cx_func_ret *r = cx_vec_start(&imp->rets);
       r != cx_vec_end(&imp->rets);
       r++) {
    if (r->type) { *(struct cx_type **)cx_vec_push(out) = r->type; }
  }
}

cx_op_type(CX_ORETURN, {
    type.eval = return_eval;
    type.emit = return_emit;
    type.emit_func = return_emit_func;
    type.emit_fimp = return_emit_fimp;
    type.emit_types = return_emit_types;
  });

static bool stash_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_vect *out = cx_vect_new();
  out->imp = s->stack;
  cx_vec_init(&s->stack, sizeof(struct cx_box));
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

static bool stop_emit(struct cx_op *op,
		      struct cx_bin *bin,
		      FILE *out,
		      struct cx *cx) {
  fputs("cx->stop = true;\n", out);
  return true;
}

cx_op_type(CX_OSTOP, {
    type.eval = stop_eval;
    type.emit = stop_emit;
  });
