#include <string.h>

#include "cixl/cmp.h"
#include "cixl/types/int.h"

enum cx_cmp cx_cmp_char(const void *x, const void *y) {
  char xv = *(const char *)x, yv = *(const char *)y;
  if (xv < yv) { return CX_CMP_LT; }
  return (xv > yv) ? CX_CMP_GT : CX_CMP_EQ;
}

enum cx_cmp cx_cmp_int(const void *x, const void *y) {
  cx_int_t xv = *(cx_int_t *)x, yv = *(cx_int_t *)y;
  if (xv < yv) { return CX_CMP_LT; }
  return (xv > yv) ? CX_CMP_GT : CX_CMP_EQ;
}

enum cx_cmp cx_cmp_ptr(const void *x, const void *y) {
  if (x < y) { return CX_CMP_LT; }
  return (x > y) ? CX_CMP_GT : CX_CMP_EQ;
}

enum cx_cmp cx_cmp_str(const void *x, const void *y) {
  int res = strcmp(*(const char **)x, *(const char **)y);
  if (res < 0) { return CX_CMP_LT; }
  return res ? CX_CMP_GT : CX_CMP_EQ;
}
