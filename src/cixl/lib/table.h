#ifndef CX_LIB_TABLE_H
#define CX_LIB_TABLE_H

#include "cixl/box.h"

struct cx;
struct cx_lib;

struct cx_table {
  struct cx *cx;
  struct cx_set entries;
  unsigned int nrefs;
};

struct cx_table_entry {
  struct cx_box key, val;
}; 

struct cx_table *cx_table_new(struct cx *cx);
struct cx_table *cx_table_ref(struct cx_table *table);
void cx_table_deref(struct cx_table *table);

struct cx_table_entry *cx_table_get(struct cx_table *table, struct cx_box *key);
void cx_table_put(struct cx_table *table, struct cx_box *key, struct cx_box *val);
bool cx_table_delete(struct cx_table *table, struct cx_box *key);

struct cx_lib *cx_init_table(struct cx *cx);
struct cx_lib *cx_init_table_types(struct cx *cx);

#endif
