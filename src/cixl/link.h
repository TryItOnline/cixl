#ifndef CX_LINK_H
#define CX_LINK_H

struct cx;

struct cx_link {
  struct cx *cx;
  char *id;
  void *handle;
};

struct cx_link *cx_link_init(struct cx_link *l,
			     struct cx *cx,
			     const char *id,
			     void *handle);

struct cx_link *cx_link_deinit(struct cx_link *l);

#endif
