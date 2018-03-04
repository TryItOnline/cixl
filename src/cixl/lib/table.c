#include <ctype.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/fimp.h"
#include "cixl/iter.h"
#include "cixl/lib.h"
#include "cixl/lib/pair.h"
#include "cixl/lib/table.h"
#include "cixl/scope.h"
#include "cixl/tok.h"

struct cx_table_iter {
  struct cx_iter iter;
  struct cx_table *table;
  size_t i;
};

bool table_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_table_iter *it = cx_baseof(iter, struct cx_table_iter, iter);

  if (it->i < it->table->entries.members.count) {
    struct cx_table_entry *e = cx_vec_get(&it->table->entries.members, it->i);
    cx_box_init(out, cx->pair_type)->as_pair = cx_pair_new(cx, &e->key, &e->val);
    it->i++;
    return true;
  }

  iter->done = true;
  return false;
}

void *table_deinit(struct cx_iter *iter) {
  struct cx_table_iter *it = cx_baseof(iter, struct cx_table_iter, iter);
  cx_table_deref(it->table);
  return it;
}

cx_iter_type(table_iter, {
    type.next = table_next;
    type.deinit = table_deinit;
  });

struct cx_table_iter *cx_table_iter_new(struct cx_table *table) {
  struct cx_table_iter *it = malloc(sizeof(struct cx_table_iter));
  cx_iter_init(&it->iter, table_iter());
  it->table = cx_table_ref(table);
  it->i = 0;
  return it;
}

struct cx_table *cx_table_new(struct cx *cx) {
  struct cx_table *t = cx_malloc(&cx->table_alloc);
  t->cx = cx;
  cx_set_init(&t->entries, sizeof(struct cx_table_entry), cx_cmp_box);
  t->entries.key_offs = offsetof(struct cx_table_entry, key);
  t->nrefs = 1;
  return t;
}

struct cx_table *cx_table_ref(struct cx_table *table) {
  table->nrefs++;
  return table;
}

void cx_table_deref(struct cx_table *table) {
  cx_test(table->nrefs);
  table->nrefs--;
  
  if (!table->nrefs) {
    cx_do_set(&table->entries, struct cx_table_entry, e) {
      cx_box_deinit(&e->key);
      cx_box_deinit(&e->val);
    }
    
    cx_set_deinit(&table->entries);
    cx_free(&table->cx->table_alloc, table);
  }
}

struct cx_table_entry *cx_table_get(struct cx_table *table, struct cx_box *key) {
  return cx_set_get(&table->entries, key);
}

void cx_table_put(struct cx_table *table, struct cx_box *key, struct cx_box *val) {
  struct cx_table_entry *e = cx_table_get(table, key);

  if (e) {
    cx_box_deinit(&e->val);
  } else {
    e = cx_set_insert(&table->entries, key);
    cx_copy(&e->key, key);
  }
  
  cx_copy(&e->val, val);
}

bool cx_table_delete(struct cx_table *table, struct cx_box *key) {
  void *found = false;
  size_t i = cx_set_find(&table->entries, key, 0, &found);
  if (!found) { return false; }
  struct cx_table_entry *e = found;
  cx_box_deinit(&e->key);
  cx_box_deinit(&e->val);
  cx_vec_delete(&table->entries.members, i);
  return true;
}

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

cx_lib(cx_init_table, "cx/table", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");
    cx_use(cx, "cx/iter/types");
    cx_use(cx, "cx/table/types");
    
    cx_add_cfunc(lib, "get",
		 cx_args(cx_arg("tbl", cx->table_type), cx_arg("key", cx->cmp_type)),
		 cx_args(cx_arg(NULL, cx->opt_type)),
		 get_imp);

    cx_add_cfunc(lib, "put",
		 cx_args(cx_arg("tbl", cx->table_type),
			 cx_arg("key", cx->cmp_type),
			 cx_arg("val", cx->any_type)),
		 cx_args(),
		 put_imp);

    cx_add_cfunc(lib, "put-else",
		 cx_args(cx_arg("tbl", cx->table_type),
			 cx_arg("key", cx->cmp_type),
			 cx_arg("upd", cx->any_type),
			 cx_arg("ins", cx->any_type)),
		 cx_args(),
		 put_else_imp);

    cx_add_cfunc(lib, "delete",
		 cx_args(cx_arg("tbl", cx->table_type), cx_arg("key", cx->cmp_type)),
		 cx_args(),
		 delete_imp);

    cx_add_cfunc(lib, "len",
		 cx_args(cx_arg("tbl", cx->table_type)),
		 cx_args(cx_arg(NULL, cx->int_type)),
		 len_imp);

    cx_add_cfunc(lib, "table",
		 cx_args(cx_arg("in", cx->seq_type)),
		 cx_args(cx_arg(NULL, cx->table_type)),
		 seq_imp);
  })

static void new_imp(struct cx_box *out) {
  out->as_table = cx_table_new(out->type->cx);
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_table == y->as_table;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  struct cx_table *xt = x->as_table, *yt = y->as_table;
  if (xt->entries.members.count != yt->entries.members.count) { return false; }
  
  for (size_t i = 0; i < xt->entries.members.count; i++) {
    struct cx_table_entry
      *xe = cx_vec_get(&xt->entries.members, i),
      *ye = cx_vec_get(&yt->entries.members, i);
    
    if (!cx_eqval(&xe->key, &ye->key) || !cx_eqval(&xe->val, &ye->val)) {
      return false;
    }
  }
  
  return true;
}

static bool ok_imp(struct cx_box *v) {
  return v->as_table->entries.members.count;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_table = cx_table_ref(src->as_table);
}

static void clone_imp(struct cx_box *dst, struct cx_box *src) {
  struct cx_table *src_tbl = src->as_table, *dst_tbl = cx_table_new(src->type->cx);
  dst->as_table = dst_tbl;

  cx_do_set(&src_tbl->entries, struct cx_table_entry, se) {
    struct cx_table_entry *de = cx_test(cx_set_insert(&dst_tbl->entries, &se->key));
    cx_clone(&de->key, &se->key);
    cx_clone(&de->val, &se->val);
  }
}

static struct cx_iter *iter_imp(struct cx_box *v) {
  return &cx_table_iter_new(v->as_table)->iter;
}

static void write_imp(struct cx_box *v, FILE *out) {
  fputs("Table new", out);
  struct cx_table *t = v->as_table;
  
  cx_do_set(&t->entries, struct cx_table_entry, e) {
    fputs(" %% ", out);
    cx_write(&e->key, out);
    fputc(' ', out);
    cx_write(&e->val, out);
    fputs(" put", out);
  }
}

static void dump_imp(struct cx_box *v, FILE *out) {
  struct cx_table *t = v->as_table;
  fputs("Table(", out);
  char sep = 0;
  
  cx_do_set(&t->entries, struct cx_table_entry, e) {
    if (sep) { fputc(sep, out); }
    fputc('(', out);
    cx_dump(&e->key, out);
    fputc(' ', out);
    cx_dump(&e->val, out);
    fputc(')', out);
    sep = ' ';
  }

  fprintf(out, ")r%d", t->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_table_deref(v->as_table);
}

cx_lib(cx_init_table_types, "cx/table/types", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");

    struct cx_type *t = cx_add_type(lib, "Table", cx->seq_type);
    t->new = new_imp;
    t->eqval = eqval_imp;
    t->equid = equid_imp;
    t->ok = ok_imp;
    t->copy = copy_imp;
    t->clone = clone_imp;
    t->iter = iter_imp;
    t->write = write_imp;
    t->dump = dump_imp;
    t->deinit = deinit_imp;
    cx->table_type = t;
  })
