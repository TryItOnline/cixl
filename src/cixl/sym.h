#ifndef CX_SYM_H
#define CX_SYM_H

#include <stddef.h>
#include <stdint.h>

#include <cixl/cmp.h>

struct cx;
struct cx_type;

struct cx_sym {
  char *id, *emit_id;
  size_t tag;
};

struct cx_sym *cx_sym_init(struct cx_sym *sym, const char *id, size_t tag);
struct cx_sym *cx_sym_deinit(struct cx_sym *sym);

enum cx_cmp cx_cmp_sym(const void *x, const void *y);

struct cx_type *cx_init_sym_type(struct cx *cx);

#endif
