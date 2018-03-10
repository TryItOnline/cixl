#ifndef CX_VEC_H
#define CX_VEC_H

#include <stdbool.h>
#include <stddef.h>

#include "cixl/util.h"

#define _cx_do_vec(_i, vec, type, var)				\
  size_t _i = 0;						\
  for (type *var = NULL;					\
       _i < (vec)->count && (var = cx_vec_get(vec, _i));	\
       _i++)							\

#define cx_do_vec(vec, type, var)		\
  _cx_do_vec(cx_gencid(i), vec, type, var)	\

#define CX_VEC_MIN 5
#define CX_VEC_GROW 3

struct cx_malloc;

struct cx_vec {
  size_t count, capac, item_size;
  unsigned char *items;
  struct cx_malloc *alloc;
  int nrefs;
};

struct cx_vec *cx_vec_new(size_t item_size);
struct cx_vec *cx_vec_init(struct cx_vec *vec, size_t item_size);
struct cx_vec *cx_vec_deinit(struct cx_vec *vec);
void cx_vec_grow(struct cx_vec *vec, size_t capac);
void *cx_vec_start(struct cx_vec *vec);
void *cx_vec_end(struct cx_vec *vec);
void *cx_vec_get(const struct cx_vec *vec, size_t i);
void *cx_vec_push(struct cx_vec *vec);
void *cx_vec_peek(struct cx_vec *vec, size_t i);
void *cx_vec_pop(struct cx_vec *vec);
void *cx_vec_put(struct cx_vec *vec, size_t i);
void *cx_vec_insert(struct cx_vec *vec, size_t i);
void cx_vec_delete(struct cx_vec *vec, size_t i);
void cx_vec_clear(struct cx_vec *vec);

#endif
