#ifndef CX_FIMP_H
#define CX_FIMP_H

#include <cixl/vec.h>

struct cx;
struct cx_func;
struct cx_scope;
struct cx_type;

typedef bool (*cx_fimp_ptr_t)(struct cx_scope *);

struct cx_fimp {
  struct cx_lib *lib;
  struct cx_func *func;
  char *id, *emit_id;
  struct cx_vec args, rets;
  cx_fimp_ptr_t ptr;
  struct cx_vec toks;
  struct cx_bin *bin;
  struct cx_scope *scope;
  size_t start_pc, nops;
};

struct cx_fimp *cx_fimp_init(struct cx_fimp *imp,
			     struct cx_lib *lib,
			     struct cx_func *func,
			     char *id);

struct cx_fimp *cx_fimp_deinit(struct cx_fimp *imp);

ssize_t cx_fimp_score(struct cx_fimp *imp, struct cx_scope *scope, ssize_t max);
bool cx_fimp_match(struct cx_fimp *imp, struct cx_scope *scope);

bool cx_fimp_inline(struct cx_fimp *imp,
		    size_t tok_idx,
		    struct cx_bin *out,
		    struct cx *cx);

bool cx_fimp_eval(struct cx_fimp *imp, struct cx_scope *scope);
bool cx_fimp_call(struct cx_fimp *imp, struct cx_scope *scope);

struct cx_type *cx_init_fimp_type(struct cx_lib *lib);

#endif
