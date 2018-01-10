#ifndef CX_TYPE_STR_H
#define CX_TYPE_STR_H

struct cx;
struct cx_type;

struct cx_str {
  size_t len;
  int nrefs;
  char data[];
};

struct cx_str *cx_str_new(const char *data);
struct cx_str *cx_str_ref(struct cx_str *str);
void cx_str_unref(struct cx_str *str);

struct cx_type *cx_init_str_type(struct cx *cx);

#endif
