#ifndef CX_STR_H
#define CX_STR_H

struct cx;
struct cx_type;

struct cx_str {
  size_t len;
  unsigned int nrefs;
  char data[];
};

struct cx_str *cx_str_new(const char *data, size_t len);
struct cx_str *cx_str_ref(struct cx_str *str);
void cx_str_deref(struct cx_str *str);
enum cx_cmp cx_cmp_str(const void *x, const void *y);

void cx_cstr_cencode(const char *in, size_t len, FILE *out);
void cx_cstr_encode(const char *in, size_t len, FILE *out);

struct cx_type *cx_init_str_type(struct cx_lib *lib);

#endif
