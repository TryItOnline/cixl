#include <ctype.h>
#include <string.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/libs/rec.h"
#include "cixl/scope.h"
#include "cixl/tok.h"
#include "cixl/types/func.h"
#include "cixl/types/fimp.h"
#include "cixl/types/table.h"

static bool check_key_type(struct cx_table *tbl, struct cx_type *typ) {  
  if (tbl->entries.members.count) {
    struct cx_table_entry *e = cx_vec_get(&tbl->entries.members, 0);
    
    if (typ != e->key.type) {
      struct cx *cx = tbl->cx;

      cx_error(cx, cx->row, cx->col,
	       "Expected key type %s, was %s", e->key.type->id, typ->id);

      return false;
    }
  }

  return true;
}

static bool get_imp(struct cx_scope *scope) {
  struct cx_box
    key = *cx_test(cx_pop(scope, false)),
    tbl = *cx_test(cx_pop(scope, false));

  bool ok = false;
  if (!check_key_type(tbl.as_table, key.type)) { goto exit; }
  struct cx_box *v = cx_table_get(tbl.as_table, &key);

  if (v) {
    cx_copy(cx_push(scope), v);
  } else {
    cx_box_init(cx_push(scope), scope->cx->nil_type);
  }

  ok = true;
 exit:
  cx_box_deinit(&key);
  cx_box_deinit(&tbl);
  return ok;
}

static bool put_imp(struct cx_scope *scope) {
  struct cx_box
    val = *cx_test(cx_pop(scope, false)),    
    key = *cx_test(cx_pop(scope, false)),
    tbl = *cx_test(cx_pop(scope, false));

  bool ok = false;
  if (!check_key_type(tbl.as_table, key.type)) { goto exit; }
  cx_table_put(tbl.as_table, &key, &val);
  ok = true;
 exit:
  cx_box_deinit(&val);
  cx_box_deinit(&key);
  cx_box_deinit(&tbl);
  return ok;
}

static bool delete_imp(struct cx_scope *scope) {
  struct cx_box
    key = *cx_test(cx_pop(scope, false)),
    tbl = *cx_test(cx_pop(scope, false));

  bool ok = false;
  if (!check_key_type(tbl.as_table, key.type)) { goto exit; }
  cx_table_delete(tbl.as_table, &key);
  ok = true;
 exit:
  cx_box_deinit(&key);
  cx_box_deinit(&tbl);
  return ok;
}

static bool len_imp(struct cx_scope *scope) {
  struct cx_box tbl = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope),
	      scope->cx->int_type)->as_int = tbl.as_table->entries.members.count;
  cx_box_deinit(&tbl);
  return true;
}

void cx_init_table(struct cx *cx) {
  cx_add_cfunc(cx, "get", get_imp,
	       cx_arg("tbl", cx->table_type), cx_arg("key", cx->cmp_type));

  cx_add_cfunc(cx, "put", put_imp,
	       cx_arg("tbl", cx->table_type),
	       cx_arg("key", cx->cmp_type),
	       cx_arg("val", cx->any_type));

  cx_add_cfunc(cx, "delete", delete_imp,
	       cx_arg("tbl", cx->table_type), cx_arg("key", cx->cmp_type));

  cx_add_cfunc(cx, "len", len_imp,
	       cx_arg("tbl", cx->table_type));
}
