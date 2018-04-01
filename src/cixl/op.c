#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/call.h"
#include "cixl/catch.h"
#include "cixl/cx.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lambda.h"
#include "cixl/op.h"
#include "cixl/rec.h"
#include "cixl/scope.h"
#include "cixl/stack.h"
#include "cixl/str.h"
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
  type->emit_init = NULL;
  type->emit_labels = NULL;
  type->emit_funcs = NULL;
  type->emit_fimps = NULL;
  type->emit_syms = NULL;
  type->emit_types = NULL;
  type->emit_libs = NULL;
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

  if (op->as_begin.fimp) { cx_push_lib(cx, op->as_begin.fimp->lib); }  
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

    fprintf(out,
	    "%s()->scope;\n"
	    "cx_push_lib(cx, %s());\n",
	    imp->emit_id, imp->lib->emit_id);
  }

  fputs("cx_begin(cx, parent);\n", out);
  return true;
}

static void begin_emit_funcs(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_fimp *imp = op->as_begin.fimp;

  if (imp) {
    struct cx_func **ok = cx_set_insert(out, &imp->func->id);
    if (ok) { *ok = imp->func; }
  }
}

static void begin_emit_fimps(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_fimp *imp = op->as_begin.fimp;

  if (imp) {
    struct cx_fimp **ok = cx_set_insert(out, &imp);
    if (ok) { *ok = imp; }
  }  
}

cx_op_type(CX_OBEGIN, {
    type.eval = begin_eval;
    type.emit = begin_emit;
    type.emit_funcs = begin_emit_funcs;
    type.emit_fimps = begin_emit_fimps;
  });

static bool catch_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx_catch_init(cx_vec_push(&cx_scope(cx, 0)->catches),
		op->as_catch.type,
		bin,
		op->tok_idx,
		op->pc+1, op->as_catch.nops,
		cx->stop_pc);

  cx->pc += op->as_catch.nops;
  return true;
}

static bool catch_emit(struct cx_op *op,
			struct cx_bin *bin,
			FILE *out,
			struct cx *cx) {
  fprintf(out,
	  "cx_catch_init(cx_vec_push(&cx_scope(cx, 0)->catches),\n"
	  "              %s(), cx->bin, %zd, %zd, %zd, cx->stop_pc);\n",
	  op->as_catch.type->emit_id, op->tok_idx, op->pc+1, op->as_catch.nops);

  fprintf(out, "goto op%zd;\n", op->pc+op->as_catch.nops+1);
  return true;
}

static void catch_emit_labels(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  size_t
    pc = op->pc+1,
    *ok = cx_set_insert(out, &pc);
  
  if (ok) { *ok = pc; }
  pc = op->pc+op->as_catch.nops+1;  
  ok = cx_set_insert(out, &pc);
  if (ok) { *ok = pc; }
}

static void catch_emit_types(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_type
    *t = op->as_catch.type,
    **ok = cx_set_insert(out, &t);

  if (ok) { *ok = t; }
}

cx_op_type(CX_OCATCH, {
    type.eval = catch_eval;
    type.emit = catch_emit;
    type.emit_labels = catch_emit_labels;
    type.emit_types = catch_emit_types;
  });

static bool else_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_box *v = cx_pop(cx_scope(cx, 0), false);
  if (!v) { return false; }
  if (!cx_ok(v)) { cx->pc += op->as_else.nops; }
  cx_box_deinit(v);
  return true;
}

static bool else_emit(struct cx_op *op,
		      struct cx_bin *bin,
		      FILE *out,
		      struct cx *cx) {
  
  fputs("struct cx_box *v = cx_pop(cx_scope(cx, 0), false);\n"
	"if (!v) { goto exit; }\n",
	out);

  fprintf(out,
	  "if (!cx_ok(v)) {\n"
	  "  cx_box_deinit(v);\n"
	  "  goto op%zd;\n"
	  "}\n\n",
	  op->pc+op->as_else.nops+1);
  
  fputs("cx_box_deinit(v);\n", out);
  return true;
}

static void else_emit_labels(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  size_t
    pc = op->pc+op->as_else.nops+1,
    *ok = cx_set_insert(out, &pc);
  
  if (ok) { *ok = pc; }
}

cx_op_type(CX_OELSE, {
    type.eval = else_eval;
    type.emit = else_emit;
    type.emit_labels = else_emit_labels;
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
  cx->pc += op->as_fimp.imp->nops;
  return true;
}

static bool fimp_emit(struct cx_op *op,
		      struct cx_bin *bin,
		      FILE *out,
		      struct cx *cx) {
  fprintf(out, "goto op%zd;\n", op->pc+op->as_fimp.imp->nops+1);
  return true;
}

static void fimp_emit_labels(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  size_t
    pc = op->as_fimp.imp->start_pc,
    *ok = cx_set_insert(out, &pc);
  
  if (ok) { *ok = pc; }
  pc = op->pc+op->as_fimp.imp->nops+1;
  ok = cx_set_insert(out, &pc);
  if (ok) { *ok = pc; }
}

static void fimp_emit_init(struct cx_op *op,
			   struct cx_bin *bin,
			   FILE *out,
			   struct cx *cx) {
  struct cx_fimp *imp = op->as_fimp.imp;
  struct cx_sym imp_var = cx_gsym(cx, "imp");
  
  fprintf(out,
	  "cx_push_lib(cx, %s());\n"
	  "struct cx_fimp *%s = %s();\n"
	  "cx_pop_lib(cx);\n"
	  "%s->bin = cx_bin_ref(cx->bin);\n"
	  "%s->start_pc = %zd;\n"
	  "%s->nops = %zd;\n",
	  imp->lib->emit_id,
	  imp_var.id, imp->emit_id,
	  imp_var.id,
	  imp_var.id, imp->start_pc,
	  imp_var.id, imp->nops);
}

static void fimp_emit_funcs(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_func
    *func = op->as_fimp.imp->func,
    **ok = cx_set_insert(out, &func->id);

  if (ok) { *ok = func; }
}

static void fimp_emit_fimps(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_fimp
    *imp = op->as_fimp.imp,
    **ok = cx_set_insert(out, &imp);

  if (ok) { *ok = imp; }
}

cx_op_type(CX_OFIMP, {
    type.eval = fimp_eval;
    type.emit = fimp_emit;
    type.emit_init = fimp_emit_init;
    type.emit_labels = fimp_emit_labels;
    type.emit_funcs = fimp_emit_funcs;
    type.emit_fimps = fimp_emit_fimps;
  });

static bool funcdef_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_fimp *imp = op->as_funcdef.imp;
  if (!imp->scope) { imp->scope = cx_scope_ref(cx_scope(cx, 0)); }
  return true;
}

static bool funcdef_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  fprintf(out,
	  "struct cx_fimp *i = %s();\n"
	  "if (!i->scope) { i->scope = cx_scope_ref(cx_scope(cx, 0)); }\n",
	  op->as_funcdef.imp->emit_id);
  
  return true;  
}

static void funcdef_emit_init(struct cx_op *op,
			      struct cx_bin *bin,
			      FILE *out,
			      struct cx *cx) {
  struct cx_fimp *imp = op->as_funcdef.imp;
  struct cx_sym args_var = cx_gsym(cx, "args");
  
  fprintf(out,
	  "struct cx_arg %s[%zd] = {\n",
	  args_var.id, imp->args.count);

  char *sep = NULL;
  
  cx_do_vec(&imp->args, struct cx_arg, a) {
    if (sep) { fputs(sep, out); }
    sep = ",\n";
    cx_arg_emit(a, out);
  }
					     
  fputs("};\n\n", out);
  struct cx_sym rets_var = cx_gsym(cx, "rets");
  
  fprintf(out,
	  "struct cx_arg %s[%zd] = {\n",
	  rets_var.id, imp->rets.count);

  sep = NULL;
  
  cx_do_vec(&imp->rets, struct cx_arg, r) {
    if (sep) { fputs(sep, out); }
    sep = ",\n";
    cx_arg_emit(r, out);
  }
  
  fputs("};\n\n", out);
  
  fprintf(out,
	  "cx_add_func(*cx->lib, \"%s\", %zd, %s, %zd, %s);\n",
	  imp->func->id, imp->args.count, args_var.id, imp->rets.count, rets_var.id);
}

static void funcdef_emit_syms(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  void push(struct cx_sym s) {
    struct cx_sym *ok = cx_set_insert(out, &s);
    if (ok) { *ok = s; }
  }

  struct cx_fimp *imp = op->as_funcdef.imp;
  
  cx_do_vec(&imp->args, struct cx_arg, a) {
    if (a->arg_type == CX_VARG && a->value.type == cx->sym_type) {
      push(a->value.as_sym);
    }
  }

  cx_do_vec(&imp->rets, struct cx_arg, a) {
    if (a->arg_type == CX_VARG && a->value.type == cx->sym_type) {
      push(a->value.as_sym);
    }
  }
}

cx_op_type(CX_OFUNCDEF, {
    type.eval = funcdef_eval;
    type.emit = funcdef_emit;
    type.emit_init = funcdef_emit_init;
    type.emit_syms = funcdef_emit_syms;
  });

static bool funcall_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_func *func = op->as_funcall.func;
  struct cx_fimp *imp = op->as_funcall.imp;
  struct cx_scope *s = cx_scope(cx, 0);
  
  if (imp) {
    if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }
  } else {
    imp = cx_func_match(func, s);
  }
  
  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
    return false;
  }
  
  if (!imp->ptr && imp->bin == cx->bin) {
    cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, cx->pc);
    cx->pc = imp->start_pc;
    return true;
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
  fprintf(out, "struct cx_func *func = %s();\n", func->emit_id);
  fputs("struct cx_fimp *imp = ", out);
  
  if (imp) {
    fprintf(out,
	    "%s();\n\n"
	    "if (s->safe && !cx_fimp_match(imp, s)) { imp = NULL; }\n\n",
	    imp->emit_id);
  } else {
    fputs("cx_func_match(func, s);\n\n", out);
  }
  
  fputs("if (!imp) {\n"
	"  cx_error(cx, cx->row, cx->col, \"Func not applicable: %s\", func->id);\n"
	"  goto exit;\n"
	"}\n\n",
	out);

  if (imp) {
    fprintf(out,
	    "if (!imp->ptr && imp->bin == cx->bin) {\n"
	    "  cx_call_init(cx_vec_push(&cx->calls), cx->row, cx->col, imp, %zd);\n"
	    "  goto op%zd;\n"
	    "} else ",
	    op->pc+1, imp->start_pc);
  }
  
  fputs("if (!cx_fimp_call(imp, s)) { goto exit; }\n",
	out);

  return true;
}

static void funcall_emit_labels(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  size_t
    pc = op->pc+1,
    *ok = cx_set_insert(out, &pc);
  
  if (ok) { *ok = pc; }
}

static void funcall_emit_funcs(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_func
    *func = op->as_funcall.func,
    **ok = cx_set_insert(out, &func->id);

  if (ok) { *ok = func; }
}

static void funcall_emit_fimps(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_fimp *imp = op->as_funcall.imp;

  if (imp) {
    struct cx_fimp **ok = cx_set_insert(out, &imp);
    if (ok) { *ok = imp; }
  }
}

cx_op_type(CX_OFUNCALL, {
    type.eval = funcall_eval;
    type.emit = funcall_emit;
    type.emit_labels = funcall_emit_labels;
    type.emit_funcs = funcall_emit_funcs;
    type.emit_fimps = funcall_emit_fimps;
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
  fprintf(out, "struct cx_box *v = cx_get_const(cx, %s, false);\n",
	  op->as_getconst.id.emit_id);

  fputs("if (!v) { goto exit; }\n"
	"cx_copy(cx_push(cx_scope(cx, 0)), v);\n",
	out);
  
  return true;
}

static void getconst_emit_syms(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_sym
    id = op->as_getconst.id,
    *ok = cx_set_insert(out, &id);
  
  if (ok) { *ok = id; }
}

cx_op_type(CX_OGETCONST, {
    type.eval = getconst_eval;
    type.emit = getconst_emit;
    type.emit_syms = getconst_emit_syms;
  });

static bool getvar_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_sym id = op->as_getvar.id;
  struct cx_box *v = cx_get_var(s, id, false);
  if (!v) { return false; }
  cx_copy(cx_push(s), v);
  return true;
}

static bool getvar_emit(struct cx_op *op,
			struct cx_bin *bin,
			FILE *out,
			struct cx *cx) {
  fprintf(out,
	  "struct cx_scope *s = cx_scope(cx, 0);\n"
	  "struct cx_box *v = cx_get_var(s, %s, false);\n"
	  "if (!v) { goto exit; }\n"
	  "cx_copy(cx_push(s), v);\n",
	  op->as_getvar.id.emit_id);

  return true;
}

static void getvar_emit_syms(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_sym id = op->as_getvar.id;

  if (id.id[0]) {
    struct cx_sym *ok = cx_set_insert(out, &id);
    if (ok) { *ok = id; }
  }
}

cx_op_type(CX_OGETVAR, {
    type.eval = getvar_eval;
    type.emit = getvar_emit;
    type.emit_syms = getvar_emit_syms;
  });

static bool jump_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx->pc = op->as_jump.pc;
  return true;
}

static bool jump_emit(struct cx_op *op,
		      struct cx_bin *bin,
		      FILE *out,
		      struct cx *cx) {
  fprintf(out, "goto op%zd;\n", op->as_jump.pc);
  return true;
}

static void jump_emit_labels(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  size_t
    pc = op->as_jump.pc,
    *ok = cx_set_insert(out, &pc);
  
  if (ok) { *ok = pc; }
}

cx_op_type(CX_OJUMP, {
    type.eval = jump_eval;
    type.emit = jump_emit;
    type.emit_labels = jump_emit_labels;
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
  fprintf(out, "goto op%zd;\n", op->pc+op->as_lambda.nops+1);
  return true;
}

static void lambda_emit_labels(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  size_t
    pc = op->as_lambda.start_op,
    *ok = cx_set_insert(out, &pc);
  
  if (ok) { *ok = pc; }
  pc = op->pc+op->as_lambda.nops+1;
  ok = cx_set_insert(out, &pc);
  if (ok) { *ok = pc; }  
}

cx_op_type(CX_OLAMBDA, {
    type.eval = lambda_eval;
    type.emit = lambda_emit;
    type.emit_labels = lambda_emit_labels;
  });

static bool libdef_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_lib_init *i = cx_vec_get(&op->as_libdef.lib->inits, op->as_libdef.init);
  cx->pc += i->nops;
  return true;
}

static bool libdef_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  struct cx_lib *lib = op->as_libdef.lib;
  struct cx_lib_init *i = cx_vec_get(&lib->inits, op->as_libdef.init);
  fprintf(out, "goto op%zd;\n", op->pc+i->nops+1);
  return true;
}

static void libdef_emit_init(struct cx_op *op,
			      struct cx_bin *bin,
			      FILE *out,
			      struct cx *cx) {
  struct cx_lib *lib = op->as_libdef.lib;
  struct cx_lib_init *i = cx_vec_get(&lib->inits, op->as_libdef.init);
  cx_test(!i->ptr);
  
  struct cx_sym lib_var = cx_gsym(cx, "lib");
  
  fprintf(out,
	  "struct cx_lib *%s = cx_add_lib(cx, \"%s\");\n"
	  "cx_lib_push_init(%s, cx_lib_ops(cx->bin, %zd, %zd));\n",
	  lib_var.id, lib->id.id, lib_var.id, i->start_pc, i->nops);
}

static void libdef_emit_labels(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  size_t
    pc = op->pc+1,
    *ok = cx_set_insert(out, &pc);
  
  if (ok) { *ok = pc; }

  struct cx_lib_init *i = cx_vec_get(&op->as_libdef.lib->inits, op->as_libdef.init);
  pc = op->pc+i->nops+1;
  ok = cx_set_insert(out, &pc);
  if (ok) { *ok = pc; }
}

cx_op_type(CX_OLIBDEF, {
    type.eval = libdef_eval;
    type.emit = libdef_emit;
    type.emit_init = libdef_emit_init;
    type.emit_labels = libdef_emit_labels;
  });

static bool popcatch_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  return cx_pop_catch(cx_scope(cx, 0), op->as_popcatch.n);
}

static bool popcatch_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  fprintf(out,
	  "if (!cx_pop_catch(cx_scope(cx, 0), %d)) { goto exit; }\n",
	  op->as_popcatch.n);
  
  return true;
}

cx_op_type(CX_OPOPCATCH, {
    type.eval = popcatch_eval;
    type.emit = popcatch_emit;
  });

static bool poplib_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx_pop_lib(cx);
  return true;
}

static bool poplib_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  fputs("cx_pop_lib(cx);\n", out);
  return true;
}

static void poplib_emit_init(struct cx_op *op,
			     struct cx_bin *bin,
			     FILE *out,
			     struct cx *cx) {
  fputs("cx_pop_lib(cx);\n", out);
}

cx_op_type(CX_OPOPLIB, {
    type.eval = poplib_eval;
    type.emit = poplib_emit;
    type.emit_init = poplib_emit_init;
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
  return cx_box_emit(&op->as_push.value, "cx_push(cx_scope(cx, 0))", out);
}

static void push_emit_funcs(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_box *v = &op->as_push.value;

  if (v->type == cx->func_type) {
    
    struct cx_func
      *f = v->as_ptr,
      **ok = cx_set_insert(out, &f->id);
    if (ok) { *ok = v->as_ptr; }
  }
}

static void push_emit_fimps(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_box *v = &op->as_push.value;

  if (v->type == cx->fimp_type) {
    struct cx_fimp **ok = cx_set_insert(out, &v->as_ptr);
    if (ok) { *ok = v->as_ptr; }
  }
}

static void push_emit_syms(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_box *v = &op->as_push.value;

  if (v->type == cx->sym_type) {
    struct cx_sym *ok = cx_set_insert(out, &v->as_sym);
    if (ok) { *ok = v->as_sym; }
  }
}

static void push_emit_types(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_box *v = &op->as_push.value;

  if (v->type == cx->meta_type) {
    struct cx_type **ok = cx_set_insert(out, &v->as_ptr);
    if (ok) { *ok = v->as_ptr; }
  }
}

cx_op_type(CX_OPUSH, {
    type.deinit = push_deinit;
    type.eval = push_eval;
    type.emit = push_emit;
    type.emit_funcs = push_emit_funcs;
    type.emit_fimps = push_emit_fimps;
    type.emit_syms = push_emit_syms;
    type.emit_types = push_emit_types;
  });

static bool pushlib_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx_push_lib(cx, op->as_pushlib.lib);
  return true;
}

static bool pushlib_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  fprintf(out,
	  "cx_push_lib(cx, %s());\n",
	  op->as_pushlib.lib->emit_id);

  return true;
}

static void pushlib_emit_init(struct cx_op *op,
			      struct cx_bin *bin,
			      FILE *out,
			      struct cx *cx) {
  fprintf(out,
	  "cx_push_lib(cx, %s());\n",
	  op->as_pushlib.lib->emit_id);
}

static void pushlib_emit_libs(struct cx_op *op,
			      struct cx_bin *bin,
			      struct cx_set *out,
			      struct cx *cx) {
  struct cx_lib
    *lib = op->as_pushlib.lib,
    **ok = cx_set_insert(out, &lib);
  if (ok) { *ok = lib; }
}

cx_op_type(CX_OPUSHLIB, {
    type.eval = pushlib_eval;
    type.emit = pushlib_emit;
    type.emit_init = pushlib_emit_init;
    type.emit_libs = pushlib_emit_libs;
  });

static bool putargs_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  struct cx_fimp *imp = op->as_putargs.imp;
  struct cx_scope *ds = cx_scope(cx, 0), *ss = ds->stack.count ? ds : cx_scope(cx, 1);
  int nargs = imp->args.count;
  
  struct cx_box *v = cx_vec_peek(&ss->stack, 0);
  ssize_t i = ss->stack.count-1;
  
  for (struct cx_arg *a = cx_vec_peek(&imp->args, 0);
       a >= (struct cx_arg *)cx_vec_start(&imp->args);
       a--, v--, i--) {
    if (a->id || a->arg_type == CX_VARG) {
      if (a->id) { *cx_put_var(ds, a->sym_id, true) = *v; }
      cx_vec_delete(&ss->stack, i);
      nargs--;
    }
  }

  if (nargs && ds != ss) {
    struct cx_box *v = cx_vec_peek(&ss->stack, nargs-1);
    
    for (struct cx_arg *a = cx_vec_start(&imp->args);
	 a != cx_vec_end(&imp->args);
	 a++) {    
      if (!a->id) { *cx_push(ds) = *v++; }
    }
    
    ss->stack.count -= nargs;
  }
  
  return true;
}

static bool putargs_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  struct cx_fimp *imp = op->as_putargs.imp;

  fputs("struct cx_scope\n"
	"  *ds = cx_scope(cx, 0),\n"
	"  *ss = ds->stack.count ? ds : cx_scope(cx, 1);\n\n",
	out);

  int nargs = imp->args.count;
  size_t i = 0;
  
  for (struct cx_arg *a = cx_vec_peek(&imp->args, 0);
       a >= (struct cx_arg *)cx_vec_start(&imp->args);
       a--) {
    if (a->id || a->arg_type == CX_VARG) {
      if (a->id) {
	fprintf(out,
		"*cx_put_var(ds, %s, true) = "
		"*(struct cx_box *)cx_vec_peek(&ss->stack, %zd);\n",
		a->sym_id.emit_id, i);
      }

      fprintf(out,
	      "cx_vec_delete(&ss->stack, ss->stack.count-%zd);\n",
	      i+1);
      
      nargs--;
    } else {
      i++;
    }
  }

  if (nargs) {
    fputs("\n"
	  "if (ds != ss) {\n",
	  out);
    
    i = nargs-1;

    for (struct cx_arg *a = cx_vec_start(&imp->args);
	 a != cx_vec_end(&imp->args);
	 a++) {
      if (!a->id) {
	fprintf(out,
		"  *cx_push(ds) = "
		"*(struct cx_box *)cx_vec_peek(&ss->stack, %zd);\n",
		i--);
      }
    }

    fprintf(out,
	    "  ss->stack.count -= %d;\n"
	    "}\n\n",
	    nargs);
  }
  
  return true;
}

static void putargs_emit_funcs(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_func
    *func = op->as_putargs.imp->func,
    **ok = cx_set_insert(out, &func->id);

  if (ok) { *ok = func; }
}

static void putargs_emit_fimps(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_fimp
    *imp = op->as_putargs.imp,
    **ok = cx_set_insert(out, &imp);

  if (ok) { *ok = imp; }
}

static void putargs_emit_syms(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_fimp *imp = op->as_putargs.imp;

  for (struct cx_arg *a = cx_vec_peek(&imp->args, 0);
       a >= (struct cx_arg *)imp->args.items;
       a--) {
    if (a->id) {
      struct cx_sym *ok = cx_set_insert(out, &a->sym_id);
      if (ok) { *ok = a->sym_id; }
    }
  }
}

cx_op_type(CX_OPUTARGS, {
    type.eval = putargs_eval;
    type.emit = putargs_emit;
    type.emit_funcs = putargs_emit_funcs;
    type.emit_fimps = putargs_emit_fimps;
    type.emit_syms = putargs_emit_syms;
  });

static bool putconst_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  return true;
}

static bool putconst_emit(struct cx_op *op,
			struct cx_bin *bin,
			FILE *out,
			struct cx *cx) {
  struct cx_sym id = op->as_putconst.id;
  struct cx_box *v = cx_lib_get_const(op->as_putconst.lib, id, false);
  if (!v) { return false; }
  
  fprintf(out,
	  "struct cx_box *v = cx_put_const(*cx->lib, %s, false);\n",
	  id.emit_id);

  return cx_box_emit(v, "v", out);
}

static void putconst_emit_syms(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_sym
    id = op->as_putconst.id,
    *ok = cx_set_insert(out, &id);

  if (ok) { *ok = id; }
}

cx_op_type(CX_OPUTCONST, {
    type.eval = putconst_eval;
    type.emit = putconst_emit;
    type.emit_syms = putconst_emit_syms;
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

static bool putvar_emit(struct cx_op *op,
			struct cx_bin *bin,
			FILE *out,
			struct cx *cx) {
  fputs("struct cx_scope *s = cx_scope(cx, 0);\n"
	"struct cx_box *src = cx_pop(s, false);\n"
	"if (!src) { goto exit; }\n\n",
	out);

  if (op->as_putvar.type) {
    fprintf(out,
	    "if (!cx_is(src->type, %s())) {\n"
	    "  cx_error(cx, cx->row, cx->col,\n"
	    "           \"Expected type %s, actual: %%s\",\n"
	    "           src->type->id);\n\n"
	    "  goto exit;\n"
            "}\n\n",
	    op->as_putvar.type->emit_id, op->as_putvar.type->id);
  }
  
  fprintf(out,
	  "struct cx_box *dst = cx_put_var(s, %s, true);\n",
	  op->as_putvar.id.emit_id);

  fputs("if (!dst) { goto exit; }\n"
	"*dst = *src;\n",
	out);

  return true;
}

static void putvar_emit_syms(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_sym
    id = op->as_putvar.id,
    *ok = cx_set_insert(out, &id);

  if (ok) { *ok = id; }
}

static void putvar_emit_types(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_type *t = op->as_putvar.type;

  if (t) {
    struct cx_type **ok = cx_set_insert(out, &t);
    if (ok) { *ok = t; }
  }
}

cx_op_type(CX_OPUTVAR, {
    type.eval = putvar_eval;
    type.emit = putvar_emit;
    type.emit_syms = putvar_emit_syms;
    type.emit_types = putvar_emit_types;
  });

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
    size_t si = 0;
    
    if (imp->rets.count) {
      struct cx_scope *ds = cx_scope(cx, 1);
      cx_vec_grow(&ds->stack, ds->stack.count+imp->rets.count);
      struct cx_arg *r = cx_vec_start(&imp->rets);
      struct cx_box *sv = cx_vec_start(&ss->stack);
      
      for (size_t ri = 0; ri < imp->rets.count; ri++, r++) {
	if (r->arg_type == CX_VARG) {
	  cx_copy(cx_push(ds), &r->value);
	  continue;
	}

	struct cx_box *v = NULL;
	
	if (r->id) {
	  v = cx_get_var(ss, r->sym_id, false);
	  if (!v) { return false; }
	} else {
	  if (si == ss->stack.count) {
	    cx_error(cx, cx->row, cx->col, "Not enough return values on stack");
	    return false;
	  }

	  v = sv++;
	  si++;
	}
	
	if (ss->safe) {
	  struct cx_type *t = NULL;

	  switch (r->arg_type) {
	  case CX_ARG:
	    t = r->type;
	    break;
	  case CX_NARG: {
	    struct cx_arg *a = cx_vec_get(&imp->args, r->narg);
	    struct cx_box *av = cx_test(cx_get_var(ss, a->sym_id, false));
	    t = av->type;
	    break;
	  }
	  default:
	    break;
	  }
	  
	  if (!cx_is(v->type, cx_test(t))) {
	    cx_error(cx, cx->row, cx->col,
		     "Invalid return type.\nExpected %s, actual: %s",
		     t->id, v->type->id);
	    
	    return false;
	  }
	}
	
	*(struct cx_box *)cx_vec_push(&ds->stack) = *v;
      }
    }

    if (si < ss->stack.count) {
      cx_error(cx, cx->row, cx->col, "Stack not empty on return");
      return false;
    }

    cx_vec_clear(&ss->stack);
    struct cx_call *call = cx_vec_pop(&cx->calls);
    if (call->return_pc > -1) { cx->pc = call->return_pc; }
    cx_call_deinit(call);
    cx_end(cx);
    cx_pop_lib(cx);
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

  fprintf(out,
	  "  if (s->safe && !cx_fimp_match(%s(), s)) {\n"
	  "    cx_error(cx, cx->row, cx->col, \"Recall not applicable\");\n"
	  "    goto exit;\n"
	  "  }\n\n"
	  "  call->recalls--;\n"
	  "  goto op%zd;\n"
	  "} else {\n"
	  "  size_t si = 0;\n",
          imp->emit_id, op->as_return.pc+1);

  if (imp->rets.count) {
    fprintf(out,
	    "  struct cx_scope *ds = cx_scope(cx, 1);\n"
	    "  cx_vec_grow(&ds->stack, ds->stack.count+%zd);\n\n",
	    imp->rets.count);    
    
    for (struct cx_arg *r = cx_vec_start(&imp->rets);
	 r != cx_vec_end(&imp->rets);
	 r++) {
      if (r->arg_type == CX_VARG) {
	cx_box_emit(&r->value, "cx_push(ds)", out);
	continue;
      }

      fputs("  {\n", out);
      
      if (r->id) {
	fprintf(out,
		"    struct cx_box *v = cx_get_var(s, %s, false);\n"
		"    if (!v) { goto exit; }\n",
		r->sym_id.emit_id);
      } else {
	fputs("    if (si == s->stack.count) {\n"
	      "      cx_error(cx, cx->row, cx->col, "
	      "\"Not enough return values on stack\");\n"
	      "      goto exit;\n"
	      "    }\n\n"
	      "    struct cx_box *v = cx_vec_get(&s->stack, si++);\n",
	      out);
      }
      
      fputs("\n"
	    "    if (s->safe) {\n",
	    out);

      switch (r->arg_type) {
      case CX_ARG:
	fprintf(out, "     struct cx_type *t = %s();\n", r->type->emit_id);
	break;
      case CX_NARG: {
	struct cx_arg *a = cx_vec_get(&imp->args, r->narg);
	
	fprintf(out,
		"      struct cx_type *t = "
		"cx_test(cx_get_var(s, %s, false))->type;\n",
		a->sym_id.emit_id);

	break;
      }	
      default:
	break;
      }

      fputs("      if (!cx_is(v->type, t)) {\n"
	    "        cx_error(cx, cx->row, cx->col,\n"
	    "                 \"Invalid return type.\\n\"\n"
            "                 \"Expected %s, actual: %s\",\n"
	    "                 t->id, v->type->id);\n"
	    "        goto exit;\n"
	    "      }\n"
	    "    }\n\n"
	    "    *(struct cx_box *)cx_vec_push(&ds->stack) = *v;\n"
	    "  }\n\n",
	    out);
    }
  } 

  fprintf(out,
	  "  if (si < s->stack.count) {\n"
	  "    cx_error(cx, cx->row, cx->col, \"Stack not empty on return\");\n"
	  "    goto exit;\n"
	  "  }\n\n"
	  "  cx_vec_clear(&s->stack);\n"
	  "  cx_end(cx);\n"
	  "  cx_pop_lib(cx);\n"
	  "  struct cx_call *call = cx_vec_pop(&cx->calls);\n\n"
	  "  if (call->return_pc > -1) {\n"
	  "    cx->pc = call->return_pc;\n"
	  "    cx_call_deinit(call);\n"
	  "    goto *op_labels[cx->pc];\n"
	  "  }\n\n"
	  "  cx_call_deinit(call);\n"
	  "}\n");
  
  return true;  
}

static void return_emit_labels(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  size_t
    pc = op->as_return.pc+1,
    *ok = cx_set_insert(out, &pc);
  
  if (ok) { *ok = pc; }
}

static void return_emit_funcs(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_func
    *f = op->as_return.imp->func,
    **ok = cx_set_insert(out, &f->id);

  if (ok) { *ok = f; }
}

static void return_emit_fimps(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_fimp
    *f = op->as_return.imp,
    **ok = cx_set_insert(out, &f);

  if (ok) { *ok = f; }
}

static void return_emit_types(struct cx_op *op, struct cx_set *out, struct cx *cx) {
  struct cx_fimp *imp = op->as_return.imp;
  
  for (struct cx_arg *r = cx_vec_start(&imp->rets);
       r != cx_vec_end(&imp->rets);
       r++) {
    if (r->arg_type == CX_ARG) {
      struct cx_type **ok = cx_set_insert(out, &r->type);
      if (ok) { *ok = r->type; }
    }
  }
}

cx_op_type(CX_ORETURN, {
    type.eval = return_eval;
    type.emit = return_emit;
    type.emit_labels = return_emit_labels;
    type.emit_funcs = return_emit_funcs;
    type.emit_fimps = return_emit_fimps;
    type.emit_types = return_emit_types;
  });

static bool stash_eval(struct cx_op *op, struct cx_bin *bin, struct cx *cx) {
  cx_stash(cx_scope(cx, 0));
  return true;
}

static bool stash_emit(struct cx_op *op,
		       struct cx_bin *bin,
		       FILE *out,
		       struct cx *cx) {
  fputs("cx_stash(cx_scope(cx, 0));\n", out);
  return true;
}

cx_op_type(CX_OSTASH, {
    type.eval = stash_eval;
    type.emit = stash_emit;
  });

static bool typedef_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  return true;
}

static void typedef_emit_init(struct cx_op *op,
			      struct cx_bin *bin,
			      FILE *out,
			      struct cx *cx) {
  struct cx_type *t = op->as_typedef.type;
  struct cx_sym type_var = cx_gsym(cx, "type");
  
  if (cx_is(t, cx->rec_type)) {
    fprintf(out,
	    "struct cx_rec_type *%s = cx_test(cx_add_rec_type(*cx->lib, \"%s\"));\n",
	    type_var.id, t->id);

    cx_do_set(&t->parents, struct cx_type *, pt) {
      if (*pt == cx->rec_type) { continue; }
      
      fprintf(out,
	      "cx_derive_rec(%s, cx_test(cx_get_type(cx, \"%s\", false)));\n",
	      type_var.id, (*pt)->id);
    }
    
    struct cx_rec_type *rt = cx_baseof(t, struct cx_rec_type, imp);

    cx_do_set(&rt->fields, struct cx_field, f) {
      fprintf(out,
	      "cx_test(cx_add_field(%s,\n"
	      "        cx_sym(cx, \"%s\"),\n"
	      "        cx_test(cx_get_type(cx, \"%s\", false)),\n"
	      "        false));\n",
	      type_var.id, f->id.id, f->type->id);
    }
  } else if (t->trait) {
    fprintf(out,
	    "struct cx_type *%s = cx_test(cx_add_type(*cx->lib, \"%s\"));\n"
	    "%s->trait = true;\n",
	    type_var.id, t->id, type_var.id);

    cx_do_set(&t->children, struct cx_type *, ct) {
      fprintf(out,
	      "cx_derive(cx_test(cx_get_type(cx, \"%s\", false)), %s);\n",
	      (*ct)->id, type_var.id);
    }
  }
}

cx_op_type(CX_OTYPEDEF, {
    type.emit = typedef_emit;
    type.emit_init = typedef_emit_init;
  });

static bool use_emit(struct cx_op *op,
			 struct cx_bin *bin,
			 FILE *out,
			 struct cx *cx) {
  return true;
}

static void use_emit_init(struct cx_op *op,
			  struct cx_bin *bin,
			  FILE *out,
			  struct cx *cx) {
  struct cx_tok *t = cx_vec_get(&bin->toks, op->tok_idx);
  struct cx_macro_eval *e = t->as_ptr;

  cx_do_vec(&e->toks, struct cx_tok, t) {
    if (t->type == CX_TID()) {
      fprintf(out,
	      "if (!cx_use(cx, \"%s\")) { goto exit; }\n",
	      (char *)t->as_ptr);
    } else {
      struct cx_tok *tt = cx_vec_get(&t->as_vec, 0);
      fprintf(out, "if (!cx_use(cx, \"%s\"", (char *)tt->as_ptr);
      tt++;

      for (; tt != cx_vec_end(&t->as_vec); tt++) {
	fprintf(out, ", \"%s\"", (char *)tt->as_ptr);
      }
      
      fputs(")) { goto exit; }\n", out);
    }
  }
}

static void use_emit_libs(struct cx_op *op,
			  struct cx_bin *bin,
			  struct cx_set *out,
			  struct cx *cx) {
  struct cx_tok *t = cx_vec_get(&bin->toks, op->tok_idx);
  struct cx_macro_eval *e = t->as_ptr;

  cx_do_vec(&e->toks, struct cx_tok, tt) {
    if (tt->type == CX_TGROUP()) {
      tt = cx_vec_get(&tt->as_vec, 0);
    }

    struct cx_lib
      *lib = cx_test(cx_get_lib(cx, tt->as_ptr, false)),
      **ok = cx_set_insert(out, &lib);
    
    if (ok) { *ok = lib; }
  }
}

cx_op_type(CX_OUSE, {
    type.emit = use_emit;
    type.emit_init = use_emit_init;
    type.emit_libs = use_emit_libs;
  });
