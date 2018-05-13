#include <stdlib.h>
#include <string.h>

#include "cixl/pmacro.h"

struct cx_pmacro *cx_pmacro_init(struct cx_pmacro *m,
			       const char *id,
			       cx_pmacro_eval_t eval) {
  m->id = strdup(id);
  m->eval = eval;
  return m;
}

struct cx_pmacro *cx_pmacro_deinit(struct cx_pmacro *m) {
  free(m->id);
  return m;
}
