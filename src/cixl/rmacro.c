#include <stdlib.h>
#include <string.h>

#include "cixl/error.h"
#include "cixl/rmacro.h"
#include "cixl/tok.h"

struct cx_rmacro *cx_rmacro_init(struct cx_rmacro *m,
			       const char *id,
			       cx_rmacro_parse_t imp) {
  m->id = strdup(id);
  m->imp = imp;
  return m;
}

struct cx_rmacro *cx_rmacro_deinit(struct cx_rmacro *m) {
  free(m->id);
  return m;
}

struct cx_rmacro_eval *cx_rmacro_eval_new(cx_rmacro_eval_t imp) {
  struct cx_rmacro_eval *e = malloc(sizeof(struct cx_rmacro_eval));
  cx_vec_init(&e->toks, sizeof(struct cx_tok));
  e->imp = imp;
  e->nrefs = 1;
  return e;
}

struct cx_rmacro_eval *cx_rmacro_eval_ref(struct cx_rmacro_eval *e) {
  e->nrefs++;
  return e;
}

void cx_rmacro_eval_deref(struct cx_rmacro_eval *e) {
  cx_test(e->nrefs);
  e->nrefs--;

  if (!e->nrefs) {
    cx_do_vec(&e->toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&e->toks);
    free(e);
  }
}
