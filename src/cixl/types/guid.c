#include <stdbool.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/guid.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

static const char *hex = "0123456789abcdef";

void cx_guid_init(cx_guid_t id) {
  for (int i=0; i<4; i++) { id[i] = rand(); }
}

void cx_guid_fprint(cx_guid_t id, FILE *out) {
  char s[37];
  strcpy(s, "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx");
  char *p = s;
  
  void print_char(int n) {
    if (*p == '-') { p++; }
    if (*p == '4') { goto exit; }
    
    if (*p == 'y') {
      *p = hex[(n & 0x03) | 0x08];
    } else {
      *p = hex[n];
    }

  exit:
    p++;
  }

  unsigned char *c = (unsigned char *)id;
  
  for (int i=0; i<16; i++, c++) {
    print_char(*c % 16);
    print_char(*c >> 4);
  }

  fputs(s, out);
}

static void new_imp(struct cx_box *out) {
  cx_guid_init(out->as_guid);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_guid == y->as_guid;
}

static void fprint_imp(struct cx_box *v, FILE *out) {
  cx_guid_fprint(v->as_guid, out);
}

struct cx_type *cx_init_guid_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Guid", cx->any_type);
  t->new = new_imp;
  t->equid = equid_imp;
  t->fprint = fprint_imp;
  return t;
}
