#ifndef CX_CMP_H
#define CX_CMP_H

enum cx_cmp { CX_CMP_LT, CX_CMP_EQ, CX_CMP_GT };

typedef enum cx_cmp (*cx_cmp_t)(const void *x, const void *y);

enum cx_cmp cx_cmp_char(const void *x, const void *y);
enum cx_cmp cx_cmp_cstr(const void *x, const void *y);
enum cx_cmp cx_cmp_int(const void *x, const void *y);
enum cx_cmp cx_cmp_ptr(const void *x, const void *y);
enum cx_cmp cx_cmp_rat(const void *x, const void *y);
enum cx_cmp cx_cmp_size(const void *x, const void *y);

#endif
