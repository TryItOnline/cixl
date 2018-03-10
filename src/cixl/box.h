#ifndef CX_BOX_H
#define CX_BOX_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "cixl/guid.h"
#include "cixl/rat.h"
#include "cixl/sym.h"
#include "cixl/time.h"

struct cx_buf;
struct cx_file;
struct cx_func;
struct cx_iter;
struct cx_lambda;
struct cx_lib;
struct cx_pair;
struct cx_poll;
struct cx_ref;
struct cx_scope;
struct cx_str;
struct cx_table;
struct cx_type;

struct cx_box {
  struct cx_type *type;
  
  union {
    bool             as_bool;
    struct cx_buf   *as_buf;
    unsigned char    as_char;
    struct cx_file  *as_file;
    cx_guid_t        as_guid;
    int64_t          as_int;
    struct cx_iter  *as_iter;
    struct cx_lib   *as_lib;
    struct cx_pair  *as_pair;
    struct cx_poll  *as_poll;
    void            *as_ptr;
    struct cx_rat    as_rat;
    struct cx_ref   *as_ref;
    struct cx_str   *as_str;
    struct cx_sym    as_sym;
    struct cx_table *as_table;
    struct cx_time   as_time;
  };
};

struct cx_box *cx_box_new(struct cx_type *type);
struct cx_box *cx_box_init(struct cx_box *box, struct cx_type *type);
struct cx_box *cx_box_deinit(struct cx_box *box);
bool cx_box_emit(struct cx_box *box, const char *exp, FILE *out);

bool cx_eqval(struct cx_box *x, struct cx_box *y);
bool cx_equid(struct cx_box *x, struct cx_box *y);
enum cx_cmp cx_cmp(const struct cx_box *x, const struct cx_box *y);
bool cx_ok(struct cx_box *x);
bool cx_call(struct cx_box *box, struct cx_scope *scope);
struct cx_box *cx_copy(struct cx_box *dst, const struct cx_box *src);
struct cx_box *cx_clone(struct cx_box *dst, struct cx_box *src);
struct cx_iter *cx_iter(struct cx_box *box);
bool cx_write(struct cx_box *box, FILE *out);
void cx_dump(struct cx_box *box, FILE *out);
void cx_print(struct cx_box *box, FILE *out);

enum cx_cmp cx_cmp_box(const void *x, const void *y);

#endif
