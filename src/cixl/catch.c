#include "cixl/bin.h"
#include "cixl/cx.h"
#include "cixl/catch.h"
#include "cixl/lib.h"
#include "cixl/type.h"

struct cx_catch *cx_catch_init(struct cx_catch *c,
			       struct cx_type *type,
			       struct cx_bin *bin,
			       size_t tok_idx,
			       size_t start_pc, size_t nops,
			       ssize_t stop_pc) {
  c->type = type;
  c->bin = cx_bin_ref(bin);
  c->tok_idx = tok_idx;
  c->start_pc = start_pc;
  c->nops = nops;
  c->stop_pc = stop_pc;
  return c;
}

struct cx_catch *cx_catch_deinit(struct cx_catch *c) {
  cx_bin_deref(c->bin);
  return c;
}

bool cx_catch_eval(struct cx_catch *c) {
  struct cx *cx = c->type->lib->cx;
  return cx_eval(c->bin,
		 c->start_pc,
		 (c->type == cx->nil_type) ? c->start_pc+c->nops-1 : c->stop_pc,
		 cx);
}
