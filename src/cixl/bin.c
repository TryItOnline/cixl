#include "cixl/bin.h"
#include "cixl/error.h"
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

bool cx_compile(struct cx *cx, struct cx_vec *in, struct cx_bin *out) {
  if (!in->count) { return true; }
  
  size_t tok_idx = out->toks.count;
  cx_vec_grow(&out->toks, out->toks.count+in->count); 
  cx_do_vec(in, struct cx_tok, t) { cx_tok_copy(cx_vec_push(&out->toks), t); }
  size_t max = out->toks.count;
  
  while (tok_idx < max) {
    struct cx_tok *tok = cx_vec_get(&out->toks, tok_idx);
    
    if (!tok->type->compile) {
      cx_error(cx, tok->row, tok->col, "Invalid token");
      return false;
    }
    
    if ((tok_idx = tok->type->compile(tok_idx, out, cx)) == -1) { return false; }
  }

  return true;
}

