#ifndef CIXL_MALLOC_H
#define CIXL_MALLOC_H

#include <stddef.h>

struct cx_malloc_slab;

struct cx_malloc {
  size_t slab_size, slot_size;
  struct cx_malloc_slab *root;
  struct cx_malloc_slot *free;
};

struct cx_malloc *cx_malloc_init(struct cx_malloc *alloc,
				 size_t slab_size,
				 size_t slot_size);

struct cx_malloc *cx_malloc_deinit(struct cx_malloc *alloc);

void *cx_malloc(struct cx_malloc *alloc);
void cx_free(struct cx_malloc *alloc, void *ptr);

#endif
