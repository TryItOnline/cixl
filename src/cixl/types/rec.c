#include <stdlib.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/types/rec.h"

struct cx_field_value {
  struct cx_sym id;
  struct cx_box value;
}; 

static void type_deinit(struct cx_type *t) {
  struct cx_rec_type *rt = cx_baseof(t, struct cx_rec_type, imp);
  cx_set_deinit(&rt->fields);
}

struct cx_rec_type *cx_rec_type_init(struct cx_rec_type *type,
				     struct cx *cx,
				     const char *id) {
  cx_type_init(&type->imp, cx, id);
  type->imp.type_deinit = type_deinit;
  cx_derive(&type->imp, cx->rec_type);
  cx_set_init(&type->fields, sizeof(struct cx_field), cx_cmp_str);
  type->fields.key_offs = offsetof(struct cx_field, id);
  cx_derive(&type->imp, cx->rec_type);
  return type;
}

struct cx_rec_type *cx_rec_type_new(struct cx *cx, const char *id) {
  return cx_rec_type_init(malloc(sizeof(struct cx_rec_type)), cx, id);
}

struct cx_rec_type *cx_rec_type_deinit(struct cx_rec_type *type) {
  cx_type_deinit(&type->imp);
  return type;
}
