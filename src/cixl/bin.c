#include "cixl/cx.h"
#include "cixl/bin.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/op.h"
#include "cixl/scope.h"
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
    
    if (op->type->eval &&
	(!op->type->eval(op, cx->bin, cx) || cx->errors.count)) { return false; }
  }
  
  return true;
}

struct cx_bin *cx_bin_init(struct cx_bin *bin) {
  cx_vec_init(&bin->toks, sizeof(struct cx_tok));
  cx_vec_init(&bin->ops, sizeof(struct cx_op));
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

bool cx_eval_str(struct cx *cx, const char *in) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = false;
  
  if (!cx_parse_str(cx, in, &toks)) { goto exit1; }

  if (!toks.count) {
    ok = true;
    goto exit1;
  }

  struct cx_bin *bin = cx_bin_new();
  if (!cx_compile(cx, cx_vec_start(&toks), cx_vec_end(&toks), bin)) { goto exit2; }
  if (!cx_eval(bin, 0, cx)) { goto exit2; }
  ok = true;
 exit2:
  cx_bin_deref(bin);
 exit1: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

bool cx_emit(struct cx_bin *bin, FILE *out, struct cx *cx) {
  cx_init_ops(bin);

  fputs("bool eval(struct cx *cx) {\n"
	"bool _eval(struct cx *cx) {\n"
        "  static bool init = true;\n\n",
	out);
  
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
	"    init = false;\n\n",
	out);

  for (struct cx_op *op = cx_vec_start(&bin->ops);
       op != cx_vec_end(&bin->ops);
       op++) {
    if (op->type->emit_init) {
      fputs("    {\n", out);
      op->type->emit_init(op, bin, out, cx);
      fputs("    }\n\n", out);
    }
  }
  
  cx_do_set(&funcs, struct cx_func *, f) {
    fprintf(out, "    %s = cx_test(cx_get_func(cx->lib, \"%s\", false));\n",
	    (*f)->emit_id, (*f)->id);
  }

  cx_do_set(&fimps, struct cx_fimp *, f) {
    fprintf(out, "    %s = cx_test(cx_get_fimp(%s, \"%s\", false));\n",
	    (*f)->emit_id, (*f)->func->emit_id, (*f)->id);
  }
  
  cx_do_set(&syms, struct cx_sym, s) {
    fprintf(out, "    %s = cx_sym(cx, \"%s\");\n", s->emit_id, s->id);
  }

  cx_do_set(&types, struct cx_type *, t) {
    fprintf(out, "    %s = cx_test(cx_get_type(cx->lib, \"%s\", false));\n",
	    (*t)->emit_id, (*t)->id);
  }
  
  cx_set_deinit(&funcs);
  cx_set_deinit(&fimps);
  cx_set_deinit(&syms);
  cx_set_deinit(&types);
		
  fputs("  }\n\n", out);

  fprintf(out, "  static void *op_labels[%zd] = {\n    ", bin->ops.count+1);
  
  for (size_t i = 0; i < bin->ops.count+1; i++) {
    fprintf(out, "&&op%zd", i);
    if (i < bin->ops.count) { fputs(", ", out); }
    if (i >= 10 && i % 10 == 0) { fputs("\n    ", out); }
  }
  
  fputs("};\n\n"
	"  goto *op_labels[cx->pc];\n\n",
	out);
  
  for (struct cx_op *op = cx_vec_start(&bin->ops);
       op != cx_vec_end(&bin->ops);
       op++) {
    cx->row = op->row; cx->col = op->col;
    struct cx_tok *tok = cx_vec_get(&bin->toks, op->tok_idx);
    
    fprintf(out, " op%zd: { /* %s %s */\n",
	    op->pc, tok->type->id, op->type->id);
    
    fprintf(out,
	    "    cx->pc = %zd; cx->row = %d; cx->col = %d;\n",
	    op->pc, cx->row, cx->col);

    fputs("    if (cx->stop) { return true; }\n", out);
    fputs("    if (cx->errors.count) { return false; }\n", out);

    if (op->type->emit && !cx_test(op->type->emit)(op, bin, out, cx)) {
      return false;
    }
    
    fputs(" }\n\n", out);
  }

  fprintf(out, " op%zd:\n",  bin->ops.count);
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

static void new_imp(struct cx_box *out) {
  out->as_ptr = cx_bin_new();
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  return cx_eval(value->as_ptr, 0, scope->cx);
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  struct cx_bin *b = src->as_ptr;
  dst->as_ptr = cx_bin_ref(b);
}

static void dump_imp(struct cx_box *value, FILE *out) {
  struct cx_bin *b = value->as_ptr;
  fprintf(out, "Bin(%p)r%d", b, b->nrefs);
}

static void deinit_imp(struct cx_box *value) {
  struct cx_bin *b = value->as_ptr;
  cx_bin_deref(b);
}

struct cx_type *cx_init_bin_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Bin", cx->any_type);
  t->new = new_imp;
  t->equid = equid_imp;
  t->call = call_imp;
  t->copy = copy_imp;
  t->dump = dump_imp;
  t->deinit = deinit_imp;
  return t;
}
