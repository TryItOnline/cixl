#include <ctype.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/fimp.h"
#include "cixl/iter.h"
#include "cixl/lib.h"
#include "cixl/lib/table.h"
#include "cixl/pair.h"
#include "cixl/scope.h"
#include "cixl/table.h"
#include "cixl/tok.h"

static bool check_key_type(struct cx_table *tbl, struct cx_type *typ) {  
  if (tbl->entries.members.count) {
    struct cx_table_entry *e = cx_vec_get(&tbl->entries.members, 0);
    
    if (!cx_is(typ, e->key.type)) {
      struct cx *cx = tbl->cx;

      cx_error(cx, cx->row, cx->col,
	       "Expected key type %s, was %s", e->key.type->id, typ->id);

      return false;
    }
  }

  return true;
}

static bool get_imp(struct cx_call *call) {
  struct cx_box
    *key = cx_test(cx_call_arg(call, 1)),
    *tbl = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  if (s->safe && !check_key_type(tbl->as_table, key->type)) { return false; }
  struct cx_table_entry *e = cx_table_get(tbl->as_table, key);

  if (e) {
    cx_copy(cx_push(s), &e->val);
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }

  return true;
}

static bool put_imp(struct cx_call *call) {
  struct cx_box
    *val = cx_test(cx_call_arg(call, 2)),    
    *key = cx_test(cx_call_arg(call, 1)),
    *tbl = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  if (s->safe && !check_key_type(tbl->as_table, key->type)) { return false; }
  cx_table_put(tbl->as_table, key, val);
  return true;
}

static bool put_call_imp(struct cx_call *call) {
  struct cx_box
    *act = cx_test(cx_call_arg(call, 2)),    
    *key = cx_test(cx_call_arg(call, 1)),
    *tbl = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  if (s->safe && !check_key_type(tbl->as_table, key->type)) { return false; }
  
  struct cx_table_entry *e = cx_table_get(tbl->as_table, key);
  if (e) {
    cx_copy(cx_push(s), &e->val);
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }
  
  if (!cx_call(act, s)) { return false; }
  struct cx_box *v = cx_pop(s, false);
  if (!v) { return false; }
  cx_table_put(tbl->as_table, key, v);
  return true;
}

static bool delete_imp(struct cx_call *call) {
  struct cx_box
    *key = cx_test(cx_call_arg(call, 1)),
    *tbl = cx_test(cx_call_arg(call, 0));

  if (!check_key_type(tbl->as_table, key->type)) { return false; }
  cx_table_delete(tbl->as_table, key);
  return true;
}

static bool len_imp(struct cx_call *call) {
  struct cx_box tbl = *cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  
  cx_box_init(cx_push(s), s->cx->int_type)->as_int =
    tbl.as_table->entries.members.count;
  
  return true;
}

static bool seq_imp(struct cx_call *call) {
  struct cx_box *in = cx_test(cx_call_arg(call, 0)), it;
  struct cx_scope *s = call->scope;
  cx_iter(in, &it);
  struct cx_table *out = cx_table_new(s->cx);
  bool ok = false;
  struct cx_box p;

  while (cx_iter_next(it.as_iter, &p, s)) {
    if (!check_key_type(out, p.as_pair->x.type)) {
      cx_table_deref(out);
      goto exit;
    }
    
    cx_table_put(out, &p.as_pair->x, &p.as_pair->y);
    cx_box_deinit(&p);
  }

  struct cx_type
    *pt = cx_type_arg(it.type, 0),
    *kt = cx_type_arg(pt, 0),
    *vt = cx_type_arg(pt, 1);
  
  cx_box_init(cx_push(s), cx_type_get(s->cx->table_type, kt, vt))->as_ptr = out;
  ok = true;
 exit:
  cx_box_deinit(&it);
  return ok;
}

cx_lib(cx_init_table, "cx/table") {
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Cmp", "Int", "Opt", "Seq") ||
      !cx_use(cx, "cx/type", "new")) {
    return false;
  }

  cx->table_type = cx_init_table_type(lib);
    
  cx_add_cfunc(lib, "get",
	       cx_args(cx_arg("tbl", cx->table_type), cx_narg(cx, "key", 0, 0)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx_arg_ref(cx, 0, 1)))),
	       get_imp);

  cx_add_cfunc(lib, "put",
	       cx_args(cx_arg("tbl", cx->table_type),
		       cx_narg(cx, "key", 0, 0),
		       cx_narg(cx, "val", 0, 1)),
	       cx_args(),
	       put_imp);

  cx_add_cfunc(lib, "put-call",
	       cx_args(cx_arg("tbl", cx->table_type),
		       cx_narg(cx, "key", 0, 0),
		       cx_arg("act", cx->any_type)),
	       cx_args(),
	       put_call_imp);

  cx_add_cfunc(lib, "delete",
	       cx_args(cx_arg("tbl", cx->table_type), cx_narg(cx, "key", 0, 0)),
	       cx_args(),
	       delete_imp);

  cx_add_cfunc(lib, "len",
	       cx_args(cx_arg("tbl", cx->table_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       len_imp);

  cx_add_cfunc(lib, "table",
	       cx_args(cx_arg("in", cx_type_get(cx->seq_type, cx->pair_type))),
	       cx_args(cx_arg(NULL, cx_type_get(cx->table_type,
						cx_arg_ref(cx, 0, 0, 0),
						cx_arg_ref(cx, 0, 0, 1)))),
	       seq_imp);

  return true;
}
