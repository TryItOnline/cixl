#include <stdio.h>
#include <stdlib.h>

#include "cixl/malloc.h"
#include "cixl/util.h"

struct cx_malloc_slab {
  size_t used_slots;
  struct cx_malloc_slab *next;
  char slots[];
};

struct cx_malloc_slot {
  struct cx_malloc_slot *next;
  char ptr[];
};

static struct cx_malloc_slab *new_slab(struct cx_malloc *alloc) {
  struct cx_malloc_slab *slab = malloc(sizeof(struct cx_malloc_slab) +
				       alloc->slab_size *
				       (sizeof(struct cx_malloc_slot) +
					alloc->slot_size));
  slab->used_slots = 0;
  slab->next = alloc->root;
  alloc->root = slab;
  return slab;
}

struct cx_malloc *cx_malloc_init(struct cx_malloc *alloc,
				 size_t slab_size,
				 size_t slot_size) {
  alloc->slab_size = slab_size;
  alloc->slot_size = slot_size;
  alloc->root = NULL;
  alloc->free = NULL;
  return alloc;
}

struct cx_malloc *cx_malloc_deinit(struct cx_malloc *alloc) {
  struct cx_malloc_slab *ns = NULL;
  
  for (struct cx_malloc_slab *s = alloc->root; s; s = ns) {
    ns = s->next;
    free(s);
  }

  return alloc;
}

void *cx_malloc(struct cx_malloc *alloc) {
  if (alloc->free) {
    struct cx_malloc_slot *s = alloc->free;
    alloc->free = s->next;
    return s->ptr;
  }
  
  struct cx_malloc_slab *s = alloc->root;
  if (!s || s->used_slots == alloc->slab_size) { s = new_slab(alloc); }
  
  void *ptr =
    s->slots +
    s->used_slots * (sizeof(struct cx_malloc_slot)+alloc->slot_size) +
    sizeof(struct cx_malloc_slot);

  s->used_slots++;
  return ptr;
}

void cx_free(struct cx_malloc *alloc, void *ptr) {
  struct cx_malloc_slot *s = cx_baseof(ptr, struct cx_malloc_slot, ptr);
  s->next = alloc->free;
  alloc->free = s;
}
