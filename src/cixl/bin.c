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
    struct cx_op *op = cx_vec_get(&cx->bin->ops, cx->pc++);
    struct cx_tok *tok = cx_vec_get(&cx->bin->toks, op->tok_idx);
    cx->row = tok->row;
    cx->col = tok->col;
    
    if (!op->type->eval(op, tok, cx) || cx->errors.count) { return false; }

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
  bin->nrefs = 1;
  bin->eval = eval;
  return bin;
}

struct cx_bin *cx_bin_deinit(struct cx_bin *bin) {
  cx_do_vec(&bin->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&bin->toks);
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
  for (struct cx_op *op = cx_vec_start(&bin->ops);
       op != cx_vec_end(&bin->ops);
       op++) {
    struct cx_tok *tok = cx_vec_get(&bin->toks, op->tok_idx);
    cx->row = tok->row;
    cx->col = tok->col;

    fprintf(out, "case %zd: {\n", op->pc);
    fprintf(out, "cx->row = %d; cx->col = %d;\n", cx->row, cx->col);
    if (!cx_test(op->type->emit)(op, tok, out, cx)) { return false; }
    fputs("}\n", out);
  }

  return true;
}
