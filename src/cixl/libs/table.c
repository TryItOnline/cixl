#include <ctype.h>
#include <string.h>

#include "cixl/args.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/libs/rec.h"
#include "cixl/scope.h"
#include "cixl/tok.h"
#include "cixl/types/func.h"
#include "cixl/types/fimp.h"
#include "cixl/types/iter.h"
#include "cixl/types/pair.h"
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
  if (scope->safe && !check_key_type(tbl.as_table, key.type)) { goto exit; }
  struct cx_table_entry *e = cx_table_get(tbl.as_table, &key);

  if (e) {
    cx_copy(cx_push(scope), &e->val);
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
  if (scope->safe && !check_key_type(tbl.as_table, key.type)) { goto exit; }
  cx_table_put(tbl.as_table, &key, &val);
  ok = true;
 exit:
  cx_box_deinit(&val);
  cx_box_deinit(&key);
  cx_box_deinit(&tbl);
  return ok;
}

static bool put_else_imp(struct cx_scope *scope) {
  struct cx_box
    ins = *cx_test(cx_pop(scope, false)),    
    upd = *cx_test(cx_pop(scope, false)),    
    key = *cx_test(cx_pop(scope, false)),
    tbl = *cx_test(cx_pop(scope, false));

  bool ok = false;
  if (scope->safe && !check_key_type(tbl.as_table, key.type)) { goto exit; }
  
  struct cx_table_entry *e = cx_table_get(tbl.as_table, &key);
  if (e) { cx_copy(cx_push(scope), &e->val); }
  if (!cx_call(e ? &upd : &ins, scope)) { goto exit; }

  struct cx_box *v = cx_pop(scope, false);
  if (!v) { goto exit; }
  cx_table_put(tbl.as_table, &key, v);
  ok = true;
 exit:
  cx_box_deinit(&ins);
  cx_box_deinit(&upd);
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

static bool seq_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box in = *cx_test(cx_pop(scope, false));
  struct cx_iter *it = cx_iter(&in);
  struct cx_table *out = cx_table_new(cx);
  bool ok = false;
  struct cx_box p;
  
  while (cx_iter_next(it, &p, scope)) {
    if (!check_key_type(out, p.as_pair->x.type)) {
      cx_table_deref(out);
      goto exit;
    }
    
    cx_table_put(out, &p.as_pair->x, &p.as_pair->y);
    cx_box_deinit(&p);
  }

  cx_box_init(cx_push(scope), cx->table_type)->as_ptr = out;
  ok = true;
 exit:
  cx_box_deinit(&in);
  cx_iter_deref(it);
  return ok;
}

void cx_init_table(struct cx *cx) {
  cx_add_cfunc(cx, "get",
	       cx_args(cx_arg("tbl", cx->table_type), cx_arg("key", cx->cmp_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       get_imp);

  cx_add_cfunc(cx, "put",
	       cx_args(cx_arg("tbl", cx->table_type),
		       cx_arg("key", cx->cmp_type),
		       cx_arg("val", cx->any_type)),
	       cx_args(),
	       put_imp);

  cx_add_cfunc(cx, "put-else",
	       cx_args(cx_arg("tbl", cx->table_type),
		       cx_arg("key", cx->cmp_type),
		       cx_arg("upd", cx->any_type),
		       cx_arg("ins", cx->any_type)),
	       cx_args(),
	       put_else_imp);

  cx_add_cfunc(cx, "delete",
	       cx_args(cx_arg("tbl", cx->table_type), cx_arg("key", cx->cmp_type)),
	       cx_args(),
	       delete_imp);

  cx_add_cfunc(cx, "len",
	       cx_args(cx_arg("tbl", cx->table_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       len_imp);

  cx_add_cfunc(cx, "table",
	       cx_args(cx_arg("in", cx->seq_type)),
	       cx_args(cx_arg(NULL, cx->table_type)),
	       seq_imp);
}
