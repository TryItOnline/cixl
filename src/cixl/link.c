#include <dlfcn.h>
#include <string.h>

#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/link.h"

struct cx_link *cx_link_init(struct cx_link *l,
			     struct cx *cx,
			     const char *id,
			     void *handle) {
  l->cx = cx;
  l->id = strdup(id);
  l->handle = handle;
  return l;
}

struct cx_link *cx_link_deinit(struct cx_link *l) {
  dlclose(l->handle);
  return l;
}
