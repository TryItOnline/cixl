#include "cixl/cx.h"
#include "cixl/bin.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/op.h"
#include "cixl/tok.h"

struct cx_bin *cx_bin_new() {
  return cx_bin_init(malloc(sizeof(struct cx_bin)));
}

struct cx_bin *cx_bin_init(struct cx_bin *bin) {
  cx_vec_init(&bin->toks, sizeof(struct cx_tok));
  cx_vec_init(&bin->ops, sizeof(struct cx_op));
  cx_set_init(&bin->funcs, sizeof(struct cx_bin_func), cx_cmp_ptr);
  bin->funcs.key_offset = offsetof(struct cx_bin_func, imp);
  bin->nrefs = 1;
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

void cx_bin_unref(struct cx_bin *bin) {
  cx_test(bin->nrefs > 0);
  bin->nrefs--;
  if (!bin->nrefs) { free(cx_bin_deinit(bin)); }
}

struct cx_bin_func *cx_bin_add_func(struct cx_bin *bin,
				    struct cx_func_imp *imp,
				    size_t start_op) {
  struct cx_bin_func *f = cx_test(cx_set_insert(&bin->funcs, &imp));
  f->imp = imp;
  f->start_op = start_op;
  return f;
}

struct cx_bin_func *cx_bin_get_func(struct cx_bin *bin, struct cx_func_imp *imp) {
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
      cx_error(cx, tok->row, tok->col, "Invalid token");
      return false;
    }
    
    if ((tok_idx = tok->type->compile(out, tok_idx, cx)) == -1) { return false; }
  }

  return true;
}

bool cx_emit(struct cx *cx, struct cx_bin *bin, FILE *out) {
  fputs("bool eval(struct cx *cx, size_t start) {\n"
	"bool _eval(size_t start) {\n"
	"cx->emit_pc = start;\n"
	"struct cx_scope *scope = cx_scope(cx, 0);\n\n"
	"while (!cx->stop) {\n"
	"switch (cx->emit_pc) {\n",
	out);
  
  size_t prev_pc = cx->emit_pc;
  cx->emit_pc = 0;
  bool ok = false;
  
  cx_do_vec(&bin->ops, struct cx_op, o) {
    struct cx_tok *tok = cx_vec_get(&bin->toks, o->tok_idx);
    cx->row = tok->row;
    cx->col = tok->col;
    
    if (!o->type->emit) {
      cx_error(cx, tok->row, tok->col, "Emit not supported yet");
      goto exit;
    }
    
    if (!o->type->emit(o, tok, out, cx)) { goto exit; }
  }

  fprintf(out, "case %zd:\n", cx->emit_pc++);
  fputs("\tcx->stop = true;\n\tbreak;\n", out);
  
  fputs("}}\n\n"
	"cx->stop = false;\n"
	"return true;\n"
	"}\n\n"
	"bool (*prev)(size_t start) = cx->emit_eval;\n"
	"cx->emit_eval = _eval;\n"
	"bool ok = _eval(start);\n"
	"cx->emit_eval = prev;\n"
	"return ok;\n"
	"}",
	out);

  ok = true;
 exit:
  cx->emit_pc = prev_pc;
  return ok;
}
