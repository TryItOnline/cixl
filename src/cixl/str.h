#ifndef CX_STR_H
#define CX_STR_H

struct cx;
struct cx_type;

struct cx_str {
  size_t len;
  unsigned int nrefs;
  char data[];
};

struct cx_str *cx_str_new(const char *data);
struct cx_str *cx_str_ref(struct cx_str *str);
void cx_str_deref(struct cx_str *str);
enum cx_cmp cx_cmp_str(const void *x, const void *y);

struct cx_type *cx_init_str_type(struct cx_lib *lib);

#endif
