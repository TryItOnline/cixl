#ifndef CX_H
#define CX_H

#include "cixl/macro.h"
#include "cixl/malloc.h"
#include "cixl/parse.h"
#include "cixl/set.h"
#include "cixl/type.h"
#include "cixl/types/fimp.h"

#define CX_VERSION "0.8.7"

#define CX_LAMBDA_SLAB_SIZE 20
#define CX_PAIR_SLAB_SIZE   20
#define CX_REC_SLAB_SIZE    20
#define CX_REF_SLAB_SIZE    20
#define CX_SCOPE_SLAB_SIZE  20
#define CX_TABLE_SLAB_SIZE  20

#define cx_add_type(cx, id, ...)		\
  _cx_add_type(cx, id, ##__VA_ARGS__, NULL)	\

struct cx_func_arg;
struct cx_func_ret;
struct cx_scope;
struct cx_sym;

struct cx {
  struct cx_set separators;
  ssize_t inline_limit1, inline_limit2;

  struct cx_set types;
  struct cx_type *any_type, *bin_type, *bool_type, *char_type, *cmp_type, *file_type,
    *fimp_type, *func_type, *guid_type, *int_type, *iter_type, *lambda_type,
    *meta_type, *nil_type, *num_type, *opt_type, *pair_type, *rat_type, *rec_type,
    *ref_type, *rfile_type, *rwfile_type, *seq_type, *str_type, *sym_type,
    *table_type, *time_type, *vect_type, *wfile_type;

  uint64_t next_sym_tag, next_type_tag;
  struct cx_set syms, macros, funcs, consts;
  
  struct cx_malloc lambda_alloc, pair_alloc, rec_alloc, ref_alloc, scope_alloc,
    table_alloc;
  
  struct cx_vec scopes;
  struct cx_scope *main, **scope;
  struct cx_vec scans, calls;
  
  struct cx_bin *bin;
  struct cx_op *op;
  bool stop;
  
  int row, col;
  struct cx_vec errors;
};

struct cx *cx_init(struct cx *cx);
struct cx *cx_deinit(struct cx *cx);

void cx_add_separators(struct cx *cx, const char *cs);
bool cx_is_separator(struct cx *cx, char c);

struct cx_type *_cx_add_type(struct cx *cx, const char *id, ...);
struct cx_type *cx_vadd_type(struct cx *cx, const char *id, va_list parents);
struct cx_rec_type *cx_add_rec_type(struct cx *cx, const char *id);
struct cx_type *cx_get_type(struct cx *cx, const char *id, bool silent);

struct cx_macro *cx_add_macro(struct cx *cx, const char *id, cx_macro_parse_t imp);
struct cx_macro *cx_get_macro(struct cx *cx, const char *id, bool silent);

struct cx_fimp *cx_add_fimp(struct cx *cx,
			    const char *id,
			    int nargs, struct cx_func_arg *args,
			    int nrets, struct cx_func_ret *rets);

struct cx_fimp *cx_add_func(struct cx *cx,
			    const char *id,
			    int nargs, struct cx_func_arg *args,
			    int nrets, struct cx_func_ret *rets,
			    const char *body);

struct cx_fimp *cx_add_cfunc(struct cx *cx,
			     const char *id,
			     int nargs, struct cx_func_arg *args,
			     int nrets, struct cx_func_ret *rets,
			     cx_fimp_ptr_t ptr);
 
struct cx_func *cx_get_func(struct cx *cx, const char *id, bool silent);

struct cx_box *cx_get_const(struct cx *cx, struct cx_sym id, bool silent);
struct cx_box *cx_set_const(struct cx *cx, struct cx_sym id, bool force);

struct cx_sym cx_sym(struct cx *cx, const char *id);

struct cx_scope *cx_scope(struct cx *cx, size_t i);
void cx_push_scope(struct cx *cx, struct cx_scope *scope);
struct cx_scope *cx_pop_scope(struct cx *cx, bool silent);
struct cx_scope *cx_begin(struct cx *cx, struct cx_scope *parent);
void cx_end(struct cx *cx);

bool cx_funcall(struct cx *cx, const char *id);
bool cx_load(struct cx *cx, const char *path);

#endif
