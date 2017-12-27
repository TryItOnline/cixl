#include <stdlib.h>
#include <string.h>

#include "cixl/error.h"
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
  cx_test(capac > vec->capac);

  if (vec->capac) {
    while (vec->capac < capac) { vec->capac *= CX_VEC_GROW_FACTOR; }
  } else {
    vec->capac = capac;
  }
	 
  vec->items = realloc(vec->items, vec->capac*vec->item_size);
}

void *cx_vec_get(struct cx_vec *vec, size_t i) {
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

static void push_pop_tests() {
  struct cx_vec vec;
  cx_vec_init(&vec, sizeof(int));
  
  for (int i = 0; i < CX_VEC_TEST_MAX; i++) {
    *(int *)cx_vec_push(&vec) = i;
  }

  cx_test(vec.count == CX_VEC_TEST_MAX);
    
  for (int i = 0; i < CX_VEC_TEST_MAX; i++) {
    cx_vec_pop(&vec);
  }

  cx_test(!vec.count);
  cx_vec_deinit(&vec);
}

static void do_tests() {
  struct cx_vec vec;
  cx_vec_init(&vec, sizeof(int));

  for (int i = 0; i < CX_VEC_TEST_MAX; i++) {
    *(int *)cx_vec_push(&vec) = i;
  }

  int expected = 0;
    
  cx_do_vec(&vec, int, actual) {
    cx_test(*actual == expected++);
  }    

  cx_vec_deinit(&vec);
}

void cx_vec_tests() {
  push_pop_tests();
  do_tests();
}
