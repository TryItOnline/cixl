#include <stdbool.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/guid.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static const char *hex = "0123456789abcdef";

static int hex_int(char c) {
  switch(c) {
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    return c - '0';
  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    return c - 'a' + 10;
  default:
    break;
  }

  return -1;  
}

void cx_guid_init(cx_guid_t id) {
  for (int i=0; i<4; i++) { id[i] = rand(); }
  unsigned char *p = (unsigned char *)id;
  p[6] &= 0xf0;
}

char *cx_guid_str(cx_guid_t id, char *out) {
  strcpy(out, "xxxxxxxx-xxxx-4xxx-xxxx-xxxxxxxxxxxx");
  char *p = out;
  
  void print_char(int n) {
    if (*p == '-') { p++; }
    if (*p == '4') { goto exit; }
    *p = hex[n];
  exit:
    p++;
  }

  unsigned char *c = (unsigned char *)id;
  
  for (int i=0; i<16; i++, c++) {
    print_char(*c % 16);
    print_char(*c >> 4);
  }

  return out;
}

bool cx_guid_parse(const char *in, cx_guid_t out) {
  int i = 0;
  const char *in_c = in;
  memset(out, 0, sizeof(cx_guid_t));
  unsigned char *out_c = (unsigned char *)out;

  bool conv(char c, int mul) {
    int v = hex_int(c);
    if (v == -1) { return false; }
    *out_c += v * mul;
    return true;
  }
  
  while (*in_c && i < CX_GUID_LEN-1) {
    if (*in_c == '-') {
      in_c++;
      i++;
    }

    if (i != 14 && (!*in_c || !conv(*in_c, 1))) { return false; }
    in_c++;
    i++;

    if (i == CX_GUID_LEN-1) { break; }
      
    if (*in_c == '-') {
      in_c++;
      i++;
    }

    if (!*in_c || !conv(*in_c, 16)) { return false; }

    out_c++;
    in_c++;
    i++;
  }
  
  return true;
}

static void new_imp(struct cx_box *out) {
  cx_guid_init(out->as_guid);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  for (int i=0; i < 4; i++) {
    if (x->as_guid[i] != y->as_guid[i]) { return false; }
  }
  
  return true;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  for (int i=0; i < 4; i++) {
    enum cx_cmp c = cx_cmp_cint(x->as_guid+i, y->as_guid+i);
    if (c != CX_CMP_EQ) { return c; }
  }

  return CX_CMP_EQ;
}

static void write_imp(struct cx_box *v, FILE *out) {
  char s[CX_GUID_LEN];
  fprintf(out, "'%s' guid", cx_guid_str(v->as_guid, s));
}

static void dump_imp(struct cx_box *v, FILE *out) {
  char s[CX_GUID_LEN];
  fputs(cx_guid_str(v->as_guid, s), out);
}

struct cx_type *cx_init_guid_type(struct cx_lib *lib) {
  struct cx_type *t = cx_add_type(lib, "Guid", lib->cx->cmp_type);
  t->new = new_imp;
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->write = write_imp;
  t->dump = dump_imp;
  return t;
}
