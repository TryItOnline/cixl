#include <stdio.h>
#include <stdlib.h>

#include "cixl/error.h"
#include "cixl/util.h"

char *cx_vfmt(const char *spec, va_list args) {
  va_list size_args;
  va_copy(size_args, args);
  int len = vsnprintf(NULL, 0, spec, size_args);
  va_end(size_args);
  char *out = malloc(len+1);
  vsnprintf(out, len+1, spec, args);
  return out;
}

char *cx_fmt(const char *spec, ...) {
  va_list args;
  va_start(args, spec);
  char *res = cx_vfmt(spec, args);
  va_end(args);
  return res;
}
