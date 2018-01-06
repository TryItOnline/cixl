#include <stdlib.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/types/struct.h"

struct cx_slot_value {
  const char *id;
  struct cx_box value;
}; 

struct cx_struct_type *cx_struct_type_init(struct cx_struct_type *type,
					   struct cx *cx,
					   const char *id) {
  cx_type_init(&type->imp, cx, id);
  cx_derive(&type->imp, cx->struct_type);
  cx_set_init(&type->slot_types, sizeof(struct cx_slot_type), cx_cmp_str);
  type->slot_types.key_offs = offsetof(struct cx_slot_type, id);
  cx_derive(&type->imp, cx->struct_type);
  return type;
}

struct cx_struct_type *cx_struct_type_new(struct cx *cx, const char *id) {
  return cx_struct_type_init(malloc(sizeof(struct cx_struct_type)), cx, id);
}

struct cx_struct_type *cx_struct_type_deinit(struct cx_struct_type *type) {
  cx_type_deinit(&type->imp);
  cx_set_deinit(&type->slot_types);
  return type;
}
