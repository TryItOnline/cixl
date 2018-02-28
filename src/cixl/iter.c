#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/iter.h"
#include "cixl/scope.h"

struct cx_iter_type *cx_iter_type_init(struct cx_iter_type *type) {
  type->next = NULL;
  type->deinit = NULL;
  return type;
}

struct cx_iter *cx_iter_init(struct cx_iter *iter, struct cx_iter_type *type) {
  iter->type = type;
  iter->nrefs = 1;
  iter->done = false;
  return iter;
}

struct cx_iter *cx_iter_ref(struct cx_iter *iter) {
  iter->nrefs++;
  return iter;
}

void cx_iter_deref(struct cx_iter *iter) {
  cx_test(iter->nrefs);
  iter->nrefs--;

  if (!iter->nrefs) {
    free(iter->type->deinit(iter));
  }
}

bool cx_iter_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  return !iter->done && cx_test(iter->type->next)(iter, out, scope);
}
