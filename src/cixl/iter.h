#ifndef CX_ITER_H
#define CX_ITER_H

#define cx_iter_type(id, ...)			\
  struct cx_iter_type *id() {			\
    static struct cx_iter_type type;		\
    static bool init = true;			\
						\
    if (init) {					\
      init = false;				\
      cx_iter_type_init(&type);			\
      __VA_ARGS__;				\
    }						\
						\
    return &type;				\
  }						\

struct cx;
struct cx_box;
struct cx_iter;
struct cx_scope;
struct cx_type;

struct cx_iter_type {
  bool (*next)(struct cx_iter *, struct cx_box *, struct cx_scope *);
  void *(*deinit)(struct cx_iter *);
};

struct cx_iter_type *cx_iter_type_init(struct cx_iter_type *type);

struct cx_iter {
  struct cx_iter_type *type;
  unsigned int nrefs;
  bool done;
};

struct cx_iter *cx_iter_init(struct cx_iter *iter, struct cx_iter_type *type);
void cx_iter_free(struct cx_iter *iter);
struct cx_iter *cx_iter_ref(struct cx_iter *iter);
void cx_iter_deref(struct cx_iter *iter);
bool cx_iter_next(struct cx_iter *iter, struct cx_box *out, struct cx_scope *scope);

#endif
