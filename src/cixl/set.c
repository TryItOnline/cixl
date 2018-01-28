#include "cixl/error.h"
#include "cixl/set.h"

struct cx_set *cx_set_init(struct cx_set *set, size_t member_size, cx_cmp_t cmp) {
  cx_vec_init(&set->members, member_size);
  set->cmp = cmp;
  set->key = NULL;
  set->key_offs = 0;
  return set;
}

struct cx_set *cx_set_deinit(struct cx_set *set) {
  cx_vec_deinit(&set->members);
  return set;
}

const void *cx_set_key(const struct cx_set *set, const void *value) {
  const char *key = set->key ? set->key(value) : value;
  return key + set->key_offs;
}

size_t cx_set_find(const struct cx_set *set,
		   const void *key,
		   size_t min,
		   void **found) {
  size_t max = set->members.count;
  
  while (min < max) {
    size_t i = (max+min) / 2;
    void *v = cx_vec_get(&set->members, i);
    const void *k = cx_set_key(set, v);

    switch (set->cmp(key, k)) {
    case CX_CMP_LT:
      max = i;
      break;
    case CX_CMP_EQ:
      if (found) { *found = v; }
      return i;
    case CX_CMP_GT:
      min = i+1;
      break;
    }
  }

  return min;
}

void *cx_set_get(const struct cx_set *set, const void *key) {
  void *found = NULL;
  cx_set_find(set, key, 0, &found);
  return found;
}

ssize_t cx_set_index(struct cx_set *set, const void *key) {
  void *found = NULL;
  size_t i = cx_set_find(set, key, 0, &found);
  return found ? i : -1;
}

void *cx_set_insert(struct cx_set *set, const void *key) {
  void *found = NULL;
  size_t i = cx_set_find(set, key, 0, &found);
  if (found) { return NULL; }
  return cx_vec_insert(&set->members, i);
}

bool cx_set_delete(struct cx_set *set, const void *key) {
  void *found = false;
  size_t i = cx_set_find(set, key, 0, &found);
  if (!found) { return false; }
  cx_vec_delete(&set->members, i);
  return true;
}

void cx_set_clear(struct cx_set *set) {
  cx_vec_clear(&set->members);
}
