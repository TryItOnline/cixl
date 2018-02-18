#include <stdlib.h>
#include <string.h>

#include "cixl/error.h"
#include "cixl/util.h"
#include "cixl/vec.h"

struct cx_vec *cx_vec_new(size_t item_size) {
  return cx_vec_init(malloc(sizeof(struct cx_vec)), item_size);
}

struct cx_vec *cx_vec_init(struct cx_vec *vec, size_t item_size) {
  vec->item_size = item_size;
  vec->count = vec->capac = 0;
  vec->items = NULL;
  return vec;
}

struct cx_vec *cx_vec_deinit(struct cx_vec *vec) {
  if (vec->items) { free(vec->items); }
  return vec;
}

void cx_vec_grow(struct cx_vec *vec, size_t capac) {
  if (capac > vec->capac) {
    if (vec->capac) {
      while (vec->capac < capac) { vec->capac *= CX_VEC_GROW; }
    } else {
      vec->capac = cx_max(capac, CX_VEC_MIN);
    }
    
    vec->items = realloc(vec->items, vec->capac*vec->item_size);
  }
}

void *cx_vec_start(struct cx_vec *vec) {
  return vec->items;
}

void *cx_vec_end(struct cx_vec *vec) {
  return vec->items ? vec->items+vec->count*vec->item_size : NULL;
}

void *cx_vec_get(const struct cx_vec *vec, size_t i) {
  return vec->items + i*vec->item_size;
}

void *cx_vec_push(struct cx_vec *vec) {
  if (vec->capac == vec->count) { cx_vec_grow(vec, vec->capac+1); }
  return cx_vec_get(vec, vec->count++);
}

void *cx_vec_peek(struct cx_vec *vec, size_t i) {
  cx_test(vec->count > i);
  return cx_vec_get(vec, vec->count-i-1);
}

void *cx_vec_pop(struct cx_vec *vec) {
  cx_test(vec->count);
  vec->count--;
  return cx_vec_get(vec, vec->count);
}

void *cx_vec_put(struct cx_vec *vec, size_t i) {
  if (vec->capac <= i) { cx_vec_grow(vec, i+1); }

  if (i > vec->count) {
    memset(cx_vec_get(vec, vec->count), 0, (i-vec->count)*vec->item_size);
  }
  
  vec->count = cx_max(i+1, vec->count);
  return cx_vec_get(vec, i);
}

void *cx_vec_insert(struct cx_vec *vec, size_t i) {
  cx_test(i <= vec->count);
  if (vec->capac == vec->count) { cx_vec_grow(vec, vec->capac+1); }
  void *p = cx_vec_get(vec, i);
  
  if (i < vec->count) {
    memmove(cx_vec_get(vec, i+1), p, (vec->count-i) * vec->item_size);
  }
  
  vec->count++;
  return p;
}

void cx_vec_delete(struct cx_vec *vec, size_t i) {
  if (i < vec->count-1) {
    memmove(cx_vec_get(vec, i), cx_vec_get(vec, i+1),
	    (vec->count-i-1) * vec->item_size);
  }

  vec->count--;
}

void cx_vec_clear(struct cx_vec *vec) {
  vec->count = 0;
}
