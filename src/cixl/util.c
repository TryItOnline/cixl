#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

char *cx_get_dir(const char *in, char *out, size_t len) {
  const char *pos = strrchr(in, '/');
  if (!pos) { pos = strrchr(in, '\\'); }

  if (!pos) {
    out[0] = 0;
    return out;
  }
  
  strncpy(out, in, cx_min(pos-in+1, len));
  out[pos-in+1] = 0;
  return out;
}
