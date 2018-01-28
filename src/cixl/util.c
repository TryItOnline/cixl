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

size_t cx_str_dist(const char *x, const char *y) {
  const size_t x_max = strlen(x)+1, y_max = strlen(y)+1;
  
  int *d = calloc(x_max*y_max, sizeof(int));
  
  size_t get_index(size_t i, size_t j) {
    return i*y_max + j;
  }
  
  for (size_t i=1; i < x_max; i++) { d[get_index(i, 0)] = i; }
  for (size_t j=1; j < y_max; j++) { d[get_index(0, j)] = j; }

  for (size_t i=1; i < x_max; i++) {
    for (size_t j=1; j < y_max; j++) {
      int sc = (x[i-1] == y[j-1]) ? 0 : 1;
    
      d[get_index(i, j)] = cx_min(d[get_index(i-1, j)]+1,
				  cx_min(d[get_index(i, j-1)]+1,
					 d[get_index(i-1, j-1)] + sc));
    }
  }
  
  int res = d[get_index(x_max-1, y_max-1)];
  free(d);
  return res;
}

bool cx_get_line(char **out, size_t *len, FILE *in) {
  if (getline(out, len, in) == -1) { return false; }

  for (char *c = *out; c < *out+*len; c++) {
    if (!*c) {
      *(c-1) = 0;
      break;
    }
  }

  return true;
}
