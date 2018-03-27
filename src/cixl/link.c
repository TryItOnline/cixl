#include <dlfcn.h>
#include <string.h>

#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/link.h"

struct cx_link *cx_link_init(struct cx_link *l,
			     struct cx *cx,
			     const char *path,
			     void *handle) {
  l->cx = cx;
  l->path = strdup(path);
  l->handle = handle;
  return l;
}

struct cx_link *cx_link_deinit(struct cx_link *l) {
  dlclose(l->handle);
  return l;
}
