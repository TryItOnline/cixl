#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/lib.h"
#include "cixl/set.h"

struct cx_lib *cx_lib_init(struct cx_lib *lib, struct cx_sym id, cx_lib_init_t init) {
  lib->id = id;
  lib->init = init;
  lib->used = false;
  return lib;
}
