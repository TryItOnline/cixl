#include "cixl/cx.h"
#include "cixl/bin.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/types/func.h"
#include "cixl/op.h"
#include "cixl/scan.h"
#include "cixl/tok.h"

struct cx_bin *cx_bin_new() {
  return cx_bin_init(malloc(sizeof(struct cx_bin)));
}

static bool eval(struct cx *cx) {
  if (!cx->bin->ops.count) { return true; }
  
  while (cx->pc < cx->bin->ops.count && !cx->stop) {
    cx_init_ops(cx->bin);
    struct cx_op *op = cx_vec_get(&cx->bin->ops, cx->pc++);
    cx->row = op->row; cx->col = op->col;
    if (!op->type->eval(op, cx->bin, cx) || cx->errors.count) { return false; }
  }
  
  return true;
}

struct cx_bin *cx_bin_init(struct cx_bin *bin) {
  cx_vec_init(&bin->toks, sizeof(struct cx_tok));
  cx_vec_init(&bin->ops, sizeof(struct cx_op));
  cx_set_init(&bin->funcs, sizeof(struct cx_bin_func), cx_cmp_ptr);
  bin->funcs.key_offs = offsetof(struct cx_bin_func, imp);
  bin->init_offs = 0;
  bin->nrefs = 1;
  bin->eval = eval;
  return bin;
}

struct cx_bin *cx_bin_deinit(struct cx_bin *bin) {
  cx_do_vec(&bin->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&bin->toks);  

  cx_do_vec(&bin->ops, struct cx_op, o) {
    if (o->type->deinit) { o->type->deinit(o); }
  }
  
  cx_vec_deinit(&bin->ops);
  cx_set_deinit(&bin->funcs);
  return bin;
}

struct cx_bin *cx_bin_ref(struct cx_bin *bin) {
  bin->nrefs++;
  return bin;
}

void cx_bin_deref(struct cx_bin *bin) {
  cx_test(bin->nrefs);
  bin->nrefs--;
  if (!bin->nrefs) { free(cx_bin_deinit(bin)); }
}

struct cx_bin_func *cx_bin_add_func(struct cx_bin *bin,
				    struct cx_fimp *imp,
				    size_t start_pc) {
  struct cx_bin_func *f = cx_test(cx_set_insert(&bin->funcs, &imp));
  f->imp = imp;
  f->start_pc = start_pc;
  return f;
}

struct cx_bin_func *cx_bin_get_func(struct cx_bin *bin, struct cx_fimp *imp) {
  return cx_set_get(&bin->funcs, &imp);
}

bool cx_compile(struct cx *cx,
		struct cx_tok *start,
		struct cx_tok *end,
		struct cx_bin *out) {
  size_t tok_idx = out->toks.count;

  for (struct cx_tok *t = start; t != end; t++) {
    cx_tok_copy(cx_vec_push(&out->toks), t);
  }
  
  size_t stop = out->toks.count;
  
  while (tok_idx < stop) {
    struct cx_tok *tok = cx_vec_get(&out->toks, tok_idx);
    
    if (!tok->type->compile) {
      cx_error(cx, tok->row, tok->col, "Invalid token: %s", tok->type->id);
      return false;
    }
    
    if ((tok_idx = tok->type->compile(out, tok_idx, cx)) == -1) { return false; }
  }

  return true;
}

void cx_init_ops(struct cx_bin *bin) {
  if (bin->init_offs < bin->ops.count) {
    for (struct cx_op *op = cx_vec_get(&bin->ops, bin->init_offs);
	 op != cx_vec_end(&bin->ops);
	 op++) {
      struct cx_tok *tok = cx_vec_get(&bin->toks, op->tok_idx);
      op->row = tok->row; op->col = tok->col;
      if (op->type->init) { op->type->init(op, tok); }
      bin->init_offs++;
    }
  }
}

bool cx_eval(struct cx_bin *bin, size_t start_pc, struct cx *cx) {
  struct cx_bin *prev_bin = cx->bin;
  size_t prev_pc = cx->pc;
  cx->bin = bin;
  cx->pc = start_pc;
  bool ok = cx_test(bin->eval)(cx);
  cx->bin = prev_bin;
  cx->pc = prev_pc;
  cx->stop = false;
  return ok;
}

bool cx_emit(struct cx_bin *bin, FILE *out, struct cx *cx) {
  cx_init_ops(bin);

  fputs("bool eval(struct cx *cx) {\n"
	"bool _eval(struct cx *cx) {\n"
        "  static bool init = true;\n\n",
	out);

  fprintf(out, "  static void *op_labels[%zd] = {\n", bin->ops.count);
  
  for (size_t i = 0; i < bin->ops.count; i++) {
    fprintf(out, "&&op%zd", i);
    if (i < bin->ops.count-1) { fputs(", ", out); }
    if (i >= 10 && i % 10 == 0) { fputc('\n', out); }
  }
  
  fputs("};\n\n", out);
  
  struct cx_set funcs, fimps, syms, types;
  cx_set_init(&funcs, sizeof(struct cx_func *), cx_cmp_ptr);
  cx_set_init(&fimps, sizeof(struct cx_fimp *), cx_cmp_ptr);
  cx_set_init(&syms, sizeof(struct cx_sym), cx_cmp_sym);
  cx_set_init(&types, sizeof(struct cx_type *), cx_cmp_ptr);

  for (struct cx_op *op = cx_vec_start(&bin->ops);
       op != cx_vec_end(&bin->ops);
       op++) {
    if (op->type->emit_funcs) { op->type->emit_funcs(op, &funcs, cx); }
    if (op->type->emit_fimps) { op->type->emit_fimps(op, &fimps, cx); }
    if (op->type->emit_syms) { op->type->emit_syms(op, &syms, cx); }
    if (op->type->emit_types) {op->type->emit_types(op, &types, cx); }
  }  

  cx_do_set(&funcs, struct cx_func *, f) {
    fprintf(out, "  static struct cx_func *%s;\n", (*f)->emit_id);
  }

  cx_do_set(&fimps, struct cx_fimp *, f) {
    fprintf(out, "  static struct cx_fimp *%s;\n", (*f)->emit_id);
  }

  cx_do_set(&syms, struct cx_sym, s) {
    fprintf(out, "  static struct cx_sym %s;\n", s->emit_id);
  }

  cx_do_set(&types, struct cx_type *, t) {
    fprintf(out, "  static struct cx_type *%s;\n", (*t)->emit_id);
  }

  fputs("\n"
	"  if (init) {\n"
	"    init = false;\n",
	out);
  
  cx_do_set(&funcs, struct cx_func *, f) {
    fprintf(out, "    %s = cx_get_func(cx, \"%s\", false);\n",
	    (*f)->emit_id, (*f)->id);
  }

  cx_do_set(&fimps, struct cx_fimp *, f) {
    fprintf(out, "    %s = cx_get_fimp(%s, \"%s\", false);\n",
	    (*f)->emit_id, (*f)->func->emit_id, (*f)->id);
  }
  
  cx_do_set(&syms, struct cx_sym, s) {
    fprintf(out, "    %s = cx_sym(cx, \"%s\");\n", s->emit_id, s->id);
  }

  cx_do_set(&types, struct cx_type *, t) {
    fprintf(out, "    %s = cx_get_type(cx, \"%s\", false);\n",
	    (*t)->emit_id, (*t)->id);
  }
  
  cx_set_deinit(&funcs);
  cx_set_deinit(&fimps);
  cx_set_deinit(&syms);
  cx_set_deinit(&types);
		
  fputs("  }\n\n"
	"  goto *op_labels[cx->pc];\n\n",
	out);
 
  for (struct cx_op *op = cx_vec_start(&bin->ops);
       op != cx_vec_end(&bin->ops);
       op++) {
    cx->row = op->row; cx->col = op->col;
    struct cx_tok *tok = cx_vec_get(&bin->toks, op->tok_idx);
    
    fprintf(out, "  op%zd: { /* %s %s */\n",
	    op->pc, tok->type->id, op->type->id);
    
    fprintf(out,
	    "    cx->pc = %zd; cx->row = %d; cx->col = %d;\n",
	    op->pc, cx->row, cx->col);

    fputs("    if (cx->stop) { return true; }\n", out);

    if (!cx_test(op->type->emit)(op, bin, out, cx)) { return false; }
    fputs("  }\n\n", out);
  }

  fputs("  cx->stop = false;\n"
	"  return true;\n"
	"}\n\n"
	"  struct cx_bin *bin = cx_bin_new();\n"
	"  bin->eval = _eval;\n"
	"  bool ok = cx_eval(bin, 0, cx);\n"
	"  cx_bin_deref(bin);\n"
	"  return ok;\n"
	"}\n",
	out);
  
  return true;
}
