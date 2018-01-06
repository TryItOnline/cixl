#ifndef CX_SET_H
#define CX_SET_H

#include <stdio.h>

#include "cixl/cmp.h"
#include "cixl/vec.h"

#define cx_do_set(set, type, var)		\
  cx_do_vec(&(set)->members, type, var)		\

struct cx_set {
  struct cx_vec members;
  cx_cmp_t cmp;
  const void *(*key)(const void *);
  size_t key_offs;
};

struct cx_set *cx_set_init(struct cx_set *set, size_t member_size, cx_cmp_t cmp);
struct cx_set *cx_set_deinit(struct cx_set *set);
const void *cx_set_key(struct cx_set *set, const void *value);
size_t cx_set_find(struct cx_set *set, const void *key, size_t min, void **found);
void *cx_set_get(struct cx_set *set, const void *key);
ssize_t cx_set_index(struct cx_set *set, const void *key);
void *cx_set_insert(struct cx_set *set, const void *key);
bool cx_set_delete(struct cx_set *set, const void *key);
		   
#define CX_SET_TEST_MAX 100

void cx_set_tests();

#endif
