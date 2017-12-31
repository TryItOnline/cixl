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
  bin->nrefs = 1;
  return bin;
}

struct cx_bin *cx_bin_deinit(struct cx_bin *bin) {
  cx_do_vec(&bin->toks, struct cx_tok, t) { cx_tok_deinit(t); }
  cx_vec_deinit(&bin->toks);
  cx_vec_deinit(&bin->ops);
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
