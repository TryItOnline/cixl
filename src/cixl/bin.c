#include "cixl/cx.h"
#include "cixl/bin.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/op.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/tok.h"

struct cx_bin *cx_bin_new() {
  return cx_bin_init(malloc(sizeof(struct cx_bin)));
}

static bool eval(struct cx *cx, ssize_t stop_pc) {
  if (!cx->bin->ops.count) { return true; }

  ssize_t prev_stop_pc = cx->stop_pc;
  cx->stop_pc = stop_pc;
  bool ok = false;
  
  while (cx->pc < cx->bin->ops.count && cx->pc != stop_pc) {
    cx_init_ops(cx->bin);
    struct cx_op *op = cx_vec_get(&cx->bin->ops, cx->pc++);
    cx->row = op->row; cx->col = op->col;
    
    if (op->type->eval) {
      if (!op->type->eval(op, cx->bin, cx) || cx->errors.count) { goto exit; }
    }
  }

  ok = true;
 exit:
  cx->stop_pc = prev_stop_pc;
  return ok;
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
  cx_bin_clear(bin);
  cx_vec_deinit(&bin->toks);  
  cx_vec_deinit(&bin->ops);
  return bin;
}

void cx_bin_clear(struct cx_bin *bin) {
  cx_do_vec(&bin->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_clear(&bin->toks);  

  cx_do_vec(&bin->ops, struct cx_op, o) {
    if (o->type->deinit) { o->type->deinit(o); }
  }
  
  cx_vec_clear(&bin->ops);
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
    
    tok_idx = tok->type->compile(out, tok_idx, cx);
  }

  return !cx->errors.count;
}

void cx_init_ops(struct cx_bin *bin) {
  if (bin->init_offs < bin->ops.count) {
    for (struct cx_op *op = cx_vec_get(&bin->ops, bin->init_offs);
	 op != cx_vec_end(&bin->ops);
	 op++) {
      struct cx_tok *tok = bin->toks.count
	? cx_vec_get(&bin->toks, op->tok_idx)
	: NULL;
      
      if (tok) { op->row = tok->row; op->col = tok->col; }
      if (op->type->init) { op->type->init(op, tok); }
      bin->init_offs++;
    }
  }
}

bool cx_eval(struct cx_bin *bin, size_t start_pc, ssize_t stop_pc, struct cx *cx) {
  struct cx_bin *prev_bin = cx->bin;
  size_t prev_pc = cx->pc;
  cx->bin = bin;
  cx->pc = start_pc;
  bool ok = cx_test(bin->eval)(cx, stop_pc);
  cx->bin = prev_bin;
  cx->pc = prev_pc;
  return ok;
}

bool cx_eval_toks(struct cx *cx, struct cx_vec *in) {
  if (!in->count) { return true; }
  bool ok = false;
  struct cx_bin *bin = cx_bin_new();
  if (!cx_compile(cx, cx_vec_start(in), cx_vec_end(in), bin)) { goto exit; }
  if (!cx_eval(bin, 0, -1, cx)) { goto exit; }
  ok = true;
 exit:
  cx_bin_deref(bin);
  return ok;
}

bool cx_eval_str(struct cx *cx, const char *in) {
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));
  bool ok = false;
  if (!cx_parse_str(cx, in, &toks)) { goto exit; }
  ok = cx_eval_toks(cx, &toks);
 exit: {
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    return ok;
  }
}

static const void *get_func_id(const void *v) {
  struct cx_func *const *f = v;
  return &(*f)->id;
}

bool cx_emit(struct cx_bin *bin, FILE *out, struct cx *cx) {
  cx_init_ops(bin);

  fputs("bool eval(struct cx *cx) {\n"
	"bool _eval(struct cx *cx, ssize_t stop_pc) {\n"
        "  static bool init = true;\n"
	"  ssize_t prev_stop_pc = cx->stop_pc;\n"
	"  cx->stop_pc = stop_pc;\n"
	"  bool ok = false;\n\n",
	out);
  
  struct cx_set labels, libs, types, funcs, fimps, syms;
  cx_set_init(&labels, sizeof(size_t), cx_cmp_size);  
  cx_set_init(&libs, sizeof(struct cx_lib *), cx_cmp_ptr);
  cx_set_init(&types, sizeof(struct cx_type *), cx_cmp_ptr);
  cx_set_init(&funcs, sizeof(struct cx_func *), cx_cmp_cstr);
  funcs.key = get_func_id;
  cx_set_init(&fimps, sizeof(struct cx_fimp *), cx_cmp_ptr);
  cx_set_init(&syms, sizeof(struct cx_sym), cx_cmp_sym);

  *(struct cx_lib **)cx_set_insert(&libs, &cx->lobby) = cx->lobby;
  
  for (struct cx_op *op = cx_vec_start(&bin->ops);
       op != cx_vec_end(&bin->ops);
       op++) {
    if (op->type->emit_labels) {op->type->emit_labels(op, &labels, cx); }
    if (op->type->emit_libs) {op->type->emit_libs(op, bin, &libs, cx); }
    if (op->type->emit_types) {op->type->emit_types(op, &types, cx); }
    if (op->type->emit_funcs) { op->type->emit_funcs(op, &funcs, cx); }
    if (op->type->emit_fimps) { op->type->emit_fimps(op, &fimps, cx); }
    if (op->type->emit_syms) { op->type->emit_syms(op, &syms, cx); }
  }

  cx_do_set(&types, struct cx_type *, t) {
    struct cx_lib **ok = cx_set_insert(&libs, &(*t)->lib);
    if (ok) { *ok = (*t)->lib; }
  }

  cx_do_set(&funcs, struct cx_func *, f) {
    struct cx_lib **ok = cx_set_insert(&libs, &(*f)->lib);
    if (ok) { *ok = (*f)->lib; }
  }

  cx_do_set(&fimps, struct cx_fimp *, f) {
    struct cx_lib **ok = cx_set_insert(&libs, &(*f)->lib);
    if (ok) { *ok = (*f)->lib; }
  }

  cx_do_set(&syms, struct cx_sym, s) {
    fprintf(out, "  static struct cx_sym %s;\n", s->emit_id);
  }

  if (syms.members.count) { fputc('\n', out); }
  
  cx_do_set(&libs, struct cx_lib *, l) {
    fprintf(out,
	    "  struct cx_lib *%s() {\n"
	    "    static struct cx_lib *l = NULL;\n"
	    "    if (!l) { l = cx_test(cx_get_lib(cx, \"%s\", false)); }\n"
	    "    return l;\n"
	    "  }\n\n",
	    (*l)->emit_id, (*l)->id.id);
  }

  cx_do_set(&types, struct cx_type *, t) {
    fprintf(out,
	    "  struct cx_type *%s() {\n"
	    "    static struct cx_type *t = NULL;\n"
	    "    if (!t) { t = cx_test(cx_get_type(cx, \"%s\", false)); }\n"
	    "    return t;\n"
	    "  }\n\n",
	    (*t)->emit_id, (*t)->id);
  }

  cx_do_set(&funcs, struct cx_func *, f) {
    fprintf(out,
	    "  struct cx_func *%s() {\n"
	    "    static struct cx_func *f = NULL;\n"
	    "    if (!f) { f = cx_test(cx_get_func(cx, \"%s\", false)); }\n"
	    "    return f;\n"
	    "  }\n\n",
	    (*f)->emit_id, (*f)->id);
  }

  cx_do_set(&fimps, struct cx_fimp *, f) {
    fprintf(out,
	    "  struct cx_fimp *%s() {\n"
	    "    static struct cx_fimp *f = NULL;\n"
	    "    if (!f) { f = cx_test(cx_get_fimp(%s(), \"%s\", false)); }\n"
	    "    return f;\n"
	    "  }\n\n",
	    (*f)->emit_id, (*f)->func->emit_id, (*f)->id);
  }

  fputs("\n"
	"  if (init) {\n"
	"    init = false;\n",
	out);

  cx_do_vec(&cx->inits, struct cx_str *, i) {
    fprintf(out,
	    "if (!cx_init_%s(cx)) { goto exit; }\n",
	    (*i)->data);
  }
  
  fputc('\n', out);

  cx_do_set(&syms, struct cx_sym, s) {
    fprintf(out, "    %s = cx_sym(cx, \"%s\");\n", s->emit_id, s->id);
  }

  fputc('\n', out);
  
  for (struct cx_op *op = cx_vec_start(&bin->ops);
       op != cx_vec_end(&bin->ops);
       op++) {
    if (op->type->emit_init) { op->type->emit_init(op, bin, out, cx); }
  }

  cx_set_deinit(&libs);
  cx_set_deinit(&types);
  cx_set_deinit(&funcs);
  cx_set_deinit(&fimps);
  cx_set_deinit(&syms);
		
  fputs("  }\n\n", out);  
  
  fprintf(out, "  static void *op_labels[%zd] = {\n    ", bin->ops.count+1);
  
  for (size_t i = 0; i < bin->ops.count+1; i++) {
    if (i == 0 || cx_set_get(&labels, &i)) {      
      fprintf(out, "&&op%zd", i);
    } else {
      fputs("NULL", out);
    }
    
    if (i < bin->ops.count) { fputs(", ", out); }
  }
  
  fputs("};\n\n"
	"  goto *op_labels[cx->pc];\n\n",
	out);
  
  for (struct cx_op *op = cx_vec_start(&bin->ops);
       op != cx_vec_end(&bin->ops);
       op++) {
    cx->row = op->row; cx->col = op->col;
    struct cx_tok *tok = cx_vec_get(&bin->toks, op->tok_idx);

    if (op->pc == 0 || cx_set_get(&labels, &op->pc)) {
      fprintf(out, "op%zd: ", op->pc);
    }
    
    fprintf(out, "{ /* %s %s */\n", tok->type->id, op->type->id);
    
    fprintf(out,
	    "cx->pc = %zd; cx->row = %d; cx->col = %d;\n",
	    op->pc, cx->row, cx->col);

    fputs("if (cx->errors.count) { goto exit; }\n\n"
	  "if (cx->pc == stop_pc) {\n"
          "  ok = true;\n"
          "  goto exit;\n"
          "}\n\n",
	  out);

    if (op->type->emit && !cx_test(op->type->emit)(op, bin, out, cx)) {
      return false;
    }
    
    fputs("}\n\n", out);
  }

  fprintf(out, " op%zd:\n", bin->ops.count);

  fputs("  ok = true;\n"
	"exit:\n"
	"  cx->stop_pc = prev_stop_pc;\n"
	"  return ok;\n"
	"}\n\n"
	
	"  struct cx_bin *bin = cx_bin_new();\n"
	"  bin->eval = _eval;\n"
	"  bool ok = cx_eval(bin, 0, -1, cx);\n"
	"  cx_bin_deref(bin);\n"
	"  return ok;\n"
	"}\n",
	out);
  
  cx_set_deinit(&labels);
  return true;
}

static void new_imp(struct cx_box *out) {
  out->as_ptr = cx_bin_new();
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool call_imp(struct cx_box *value, struct cx_scope *scope) {
  return cx_eval(value->as_ptr, 0, -1, scope->cx);
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
