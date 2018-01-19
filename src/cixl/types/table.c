#include <stdlib.h>

#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/types/iter.h"
#include "cixl/types/table.h"

struct cx_table_iter {
  struct cx_iter iter;
  struct cx_table *table;
  size_t i;
};

bool table_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope) {
  struct cx_table_iter *it = cx_baseof(iter, struct cx_table_iter, iter);

  if (it->i < it->table->entries.members.count) {
    struct cx_table_entry *e = cx_vec_get(&it->table->entries.members, it->i);
    cx_copy(out, &e->val);
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

struct cx_box *cx_table_get(struct cx_table *table, struct cx_box *key) {
  struct cx_table_entry *e = cx_set_get(&table->entries, key);
  return e ? &e->val : NULL;
}

void cx_table_put(struct cx_table *table, struct cx_box *key, struct cx_box *val) {
  struct cx_table_entry *e = cx_set_get(&table->entries, key);

  if (e) {
    cx_box_deinit(&e->val);
  } else {
    e = cx_set_insert(&table->entries, key);
  }
  
  cx_copy(&e->key, key);
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

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
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
  fputs("(Table new", out);
  struct cx_table *t = v->as_table;
  
  cx_do_set(&t->entries, struct cx_table_entry, e) {
    fputs(" %% ", out);
    cx_write(&e->key, out);
    fputc(' ', out);
    cx_write(&e->val, out);
    fputs(" put", out);
  }

  fputc(')', out);
}

static void print_imp(struct cx_box *v, FILE *out) {
  struct cx_table *t = v->as_table;
  fputs("Table(", out);
  char sep = 0;
  
  cx_do_set(&t->entries, struct cx_table_entry, e) {
    if (sep) { fputc(sep, out); }
    fputc('(', out);
    cx_print(&e->key, out);
    fputc(' ', out);
    cx_print(&e->val, out);
    fputc(')', out);
    sep = ' ';
  }

  fprintf(out, ")@%d", t->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_table_deref(v->as_table);
}

struct cx_type *cx_init_table_type(struct cx *cx) {
  struct cx_type *t = cx_add_type(cx, "Table", cx->any_type, cx->seq_type);
  t->new = new_imp;
  t->eqval = eqval_imp;
  t->equid = equid_imp;
  t->ok = ok_imp;
  t->copy = copy_imp;
  t->clone = clone_imp;
  t->iter = iter_imp;
  t->write = write_imp;
  t->print = print_imp;
  t->deinit = deinit_imp;
  
  return t;
}
