## Slab Allocation
#### 2018-03-04

### Intro
Manual memory management comes with its share of complexity and risks; but when it's the right tool for the job, it's the right tool for the job. On the more positive side, it allows using local knowledge about the problem being solved to custom tailor the allocation scheme. One opportunity for doing so when creating large numbers of equally sized values is to allocate multiple values at once and dish them out on request, also known as slab allocation. Advantages of slab allocation include reduced fragmentation which makes malloc's job easier; stable pointers as opposed to using realloc to grow allocations, without sacrificing data locality since values allocated in sequence are still placed next to each other in memory within slabs; and gradual memory use as opposed to pre-allocating a single heap block for all values. This post describes a minimal viable implementation of the idea in C, with the added twist of recycling freed values. The implementation is taken from [Cixl](https://github.com/basic-gongfu/cixl) which uses slab allocation [extensively](https://github.com/basic-gongfu/cixl/blob/bc38f8437de6d71d8482b75188ce0d0ad5f62548/src/cixl/cx.h#L22).

Each allocator has a fixed slot size and number of slot per slab.

```C
#include <stddef.h>

struct cx_malloc_slab;

struct cx_malloc {
  size_t slab_size, slot_size;
  struct cx_malloc_slab *root;
  struct cx_malloc_slot *free;
};

struct cx_malloc *cx_malloc_init(struct cx_malloc *alloc,
				 size_t slab_size,
				 size_t slot_size) {
  alloc->slab_size = slab_size;
  alloc->slot_size = slot_size;
  alloc->root = NULL;
  alloc->free = NULL;
  return alloc;
}
```

Slabs keeps track of number of used slots and contain a link to the next slab. The latest slab is always kept as allocator root.

```C
struct cx_malloc_slab {
  size_t used_slots;
  struct cx_malloc_slab *next;
  char slots[];
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

struct cx_malloc *cx_malloc_deinit(struct cx_malloc *alloc) {
  for (struct cx_malloc_slab *s = alloc->root, *ns = NULL; s; s = ns) {
    ns = s->next;
    free(s);
  }

  return alloc;
}
```

Slots represent individual allocations and contain a pointer to the next recycled slot. This means that the size of a pointer will be added to each individual allocation, which is rarely an issue since it usually barely registers on the radar compared to the size of the value.

```C
struct cx_malloc_slot {
  struct cx_malloc_slot *next;
  char ptr[];
};
```

When allocating values, recycled values are used as a fast path before hitting the slabs. Once we have our hands on a slab, a pointer to the next free slot is generated and returned.

```C
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
```

Freeing values adds them to the recycling list using the embedded next pointer.

```C
void cx_free(struct cx_malloc *alloc, void *ptr) {
  struct cx_malloc_slot *s = cx_baseof(ptr, struct cx_malloc_slot, ptr);
  s->next = alloc->free;
  alloc->free = s;
}
```

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts. You may find more of the same kind of material [here](https://github.com/basic-gongfu/cixl/tree/master/devlog).