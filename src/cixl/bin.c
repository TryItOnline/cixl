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

    while (cx->scans.count) {
      struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
      if (!cx_scan_ok(s)) { break; }
      cx_vec_pop(&cx->scans);
      if (!cx_scan_call(s)) { return false; }
    }
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
  size_t prev_pc = cx->pc, prev_nscans = cx->scans.count;
  cx->bin = bin;
  cx->pc = start_pc;
  bool ok = cx_test(bin->eval)(cx);

  if (ok && cx->scans.count > prev_nscans) {
    struct cx_scan *s = cx_vec_peek(&cx->scans, 0);
    
    cx_error(cx, cx->row, cx->col,
	     "Not enough args for func: '%s'", s->func->id);

    cx->scans.count = prev_nscans;
    ok = false;
  }

  cx->bin = prev_bin;
  cx->pc = prev_pc;
  cx->stop = false;
  return ok;
}

bool cx_emit(struct cx_bin *bin, FILE *out, struct cx *cx) {
  cx_init_ops(bin);

  fputs("bool eval(struct cx *cx) {\n"
        "  static bool init = true;\n",
	out);

  struct cx_set func_dup;
  cx_set_init(&func_dup, sizeof(void *), cx_cmp_ptr);

  struct cx_set sym_dup;
  cx_set_init(&sym_dup, sizeof(size_t), cx_cmp_size);

  struct cx_vec syms;
  cx_vec_init(&syms, sizeof(struct cx_sym));

  struct cx_set type_dup;
  cx_set_init(&type_dup, sizeof(size_t), cx_cmp_size);

  struct cx_vec types;
  cx_vec_init(&types, sizeof(struct cx_type *));

  for (struct cx_op *op = cx_vec_start(&bin->ops);
       op != cx_vec_end(&bin->ops);
       op++) {
    if (op->type->emit_func) {
      struct cx_func *f = op->type->emit_func(op);

      if (f) {
	void **ok = cx_set_insert(&func_dup, &f);
	
	if (ok) {
	  *ok = f;
	  fprintf(out, "  static struct cx_func *func%zd = NULL;\n", f->tag);
	}
      }
    }

    if (op->type->emit_fimp) {
      struct cx_fimp *f = op->type->emit_fimp(op);

      if (f) {
	void **ok = cx_set_insert(&func_dup, &f);
      
	if (ok) {
	  *ok = f;

	  fprintf(out, "  static struct cx_fimp *fimp%zd_%zd = NULL;\n",
		  f->func->tag, f->idx);
	}
      }
    }

    if (op->type->emit_syms) {
      op->type->emit_syms(op, &syms);
      
      cx_do_vec(&syms, struct cx_sym, s) {
	size_t *ok = cx_set_insert(&sym_dup, &s->tag);

	if (ok) {
	  *ok = s->tag;
	  fprintf(out, "  static struct cx_sym sym%zd;\n", s->tag);
	}
      }

      cx_vec_clear(&syms);
    }

    if (op->type->emit_types) {
      op->type->emit_types(op, &types);
      
      cx_do_vec(&types, struct cx_type *, tp) {
	size_t
	  tag = (*tp)->tag,
	  *ok = cx_set_insert(&sym_dup, &tag);

	if (ok) {
	  *ok = tag;
	  fprintf(out, "  static struct cx_type *type%zd;\n", tag);
	}
      }

      cx_vec_clear(&types);
    }
  }  
  
  fputs("\n"
	"  if (init) {\n"
	"    init = false;\n",
	out);
  
  cx_set_clear(&func_dup);
  cx_set_clear(&sym_dup);
  cx_set_clear(&type_dup);
  
  for (struct cx_op *op = cx_vec_start(&bin->ops);
       op != cx_vec_end(&bin->ops);
       op++) {
    if (op->type->emit_func) {
      struct cx_func *f = op->type->emit_func(op);

      if (f) {
	void **ok = cx_set_insert(&func_dup, &f);
	
	if (ok) {
	  *ok = f;
	  
	  fprintf(out, "    func%zd = cx_get_func(cx, \"%s\", false);\n",
		  f->tag, f->id);
	}
      }
    }
    
    if (op->type->emit_fimp) {
      struct cx_fimp *f = op->type->emit_fimp(op);

      if (f) {
	void **ok = cx_set_insert(&func_dup, &f);
	
	if (ok) {
	  *ok = f;
	
	  fprintf(out, "    fimp%zd_%zd = cx_func_get_imp(func%zd, \"%s\", false);\n",
		  f->func->tag, f->idx, f->func->tag, f->id);
	}
      }
    }

    if (op->type->emit_syms) {
      op->type->emit_syms(op, &syms);
      
      cx_do_vec(&syms, struct cx_sym, s) {
	size_t *ok = cx_set_insert(&sym_dup, &s->tag);

	if (ok) {
	  *ok = s->tag;
	  fprintf(out, "    sym%zd = cx_sym(cx, \"%s\");\n", s->tag, s->id);
	}
      }

      cx_vec_clear(&syms);
    }

    if (op->type->emit_types) {
      op->type->emit_types(op, &types);
      
      cx_do_vec(&types, struct cx_type *, tp) {
	size_t
	  tag = (*tp)->tag,
	  *ok = cx_set_insert(&type_dup, &tag);

	if (ok) {
	  *ok = tag;
	  fprintf(out, "    type%zd = cx_get_type(cx, \"%s\", false);\n",
		  tag, (*tp)->id);
	}
      }

      cx_vec_clear(&types);
    }
  }

  cx_set_deinit(&func_dup);
  cx_set_deinit(&sym_dup);
  cx_vec_deinit(&syms);
  cx_set_deinit(&type_dup);
  cx_vec_deinit(&types);
		
  fputs("  }\n\n"
	"  while (!cx->stop) {\n"
	"    switch (cx->pc) {\n",
	out);

  
  for (struct cx_op *op = cx_vec_start(&bin->ops);
       op != cx_vec_end(&bin->ops);
       op++) {
    cx->row = op->row; cx->col = op->col;
    fprintf(out, "      case %zd: { /* %s */\n", op->pc, op->type->id);
    fprintf(out, "        cx->row = %d; cx->col = %d;\n", cx->row, cx->col);
    if (!cx_test(op->type->emit)(op, bin, out, cx)) { return false; }
    fputs("      }\n", out);
  }

  fputs("      default:\n"
	"        return true;\n"
	"    }\n\n"
	"    while (cx->scans.count) {\n"
	"      struct cx_scan *s = cx_vec_peek(&cx->scans, 0);\n"
	"      if (!cx_scan_ok(s)) { break; }\n"
	"      cx_vec_pop(&cx->scans);\n"
        "      if (!cx_scan_call(s)) { return false; }\n"
	"    }\n"
        "  }\n\n"
	"  cx->stop = false;\n"
	"  return true;\n"
	"}\n",
	out);
  
  return true;
}
