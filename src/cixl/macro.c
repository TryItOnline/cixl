#include <stdlib.h>
#include <string.h>

#include "cixl/error.h"
#include "cixl/macro.h"
#include "cixl/tok.h"

struct cx_macro *cx_macro_init(struct cx_macro *macro,
			       const char *id,
			       cx_macro_parse_t imp) {
  macro->id = strdup(id);
  macro->imp = imp;
  return macro;
}

struct cx_macro *cx_macro_deinit(struct cx_macro *macro) {
  free(macro->id);
  return macro;
}

struct cx_macro_eval *cx_macro_eval_new(cx_macro_eval_t imp) {
  struct cx_macro_eval *eval = malloc(sizeof(struct cx_macro_eval));
  cx_vec_init(&eval->toks, sizeof(struct cx_tok));
  eval->imp = imp;
  eval->nrefs = 1;
  return eval;
}

struct cx_macro_eval *cx_macro_eval_ref(struct cx_macro_eval *eval) {
  eval->nrefs++;
  return eval;
}

void cx_macro_eval_deref(struct cx_macro_eval *eval) {
  cx_test(eval->nrefs);
  eval->nrefs--;

  if (!eval->nrefs) {
    cx_do_vec(&eval->toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&eval->toks);
    free(eval);
  }
}
