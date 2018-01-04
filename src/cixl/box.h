#ifndef CX_BOX_H
#define CX_BOX_H

#include <stdbool.h>
#include <stdio.h>
#include "cixl/types/int.h"
#include "cixl/types/rat.h"
#include "cixl/types/time.h"

struct cx_func;
struct cx_lambda;
struct cx_scope;
struct cx_type;

struct cx_box {
  struct cx_type *type;
  bool undef;
  
  union {
    bool as_bool;
    char as_char;
    cx_int_t as_int;
    void *as_ptr;
    struct cx_rat as_rat;
    struct cx_time as_time;
  };
};

struct cx_box *cx_box_new(struct cx_type *type);
struct cx_box *cx_box_init(struct cx_box *box, struct cx_type *type);
struct cx_box *cx_box_deinit(struct cx_box *box);

bool cx_eqval(struct cx_box *x, struct cx_box *y);
bool cx_equid(struct cx_box *x, struct cx_box *y);
bool cx_ok(struct cx_box *x);
bool cx_call(struct cx_box *box, struct cx_scope *scope);
struct cx_box *cx_copy(struct cx_box *dst, struct cx_box *src);
void cx_fprint(struct cx_box *box, FILE *out);

#endif
