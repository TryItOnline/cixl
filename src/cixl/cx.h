#ifndef CX_H
#define CX_H

#include "cixl/macro.h"
#include "cixl/set.h"
#include "cixl/type.h"

#define CX_VERSION "0.5"

#define cx_add_func(cx, id, ...) ({				\
      struct cx_func_arg args[] = {__VA_ARGS__};		\
      int nargs = sizeof(args)/sizeof(struct cx_func_arg);	\
      _cx_add_func(cx, id, nargs, args);			\
    })								\

#define cx_add_type(cx, id, ...)		\
  _cx_add_type(cx, id, ##__VA_ARGS__, NULL)	\

struct cx_func_arg;
struct cx_scope;

struct cx {
  struct cx_set separators;

  struct cx_set types;
  struct cx_type *any_type, *bin_type, *bool_type, *char_type, *func_type, *int_type,
    *lambda_type, *meta_type, *nil_type, *opt_type, *str_type, *vect_type;

  cx_type_tag_t next_type;
  struct cx_set macros, funcs;
  
  struct cx_vec scopes;
  struct cx_scope *main;
  
  struct cx_func_imp *func_imp;

  struct cx_bin *bin;
  struct cx_op *op;  
  bool stop;
  
  int row, col;
  struct cx_vec errors;
};

struct cx *cx_init(struct cx *cx);
void cx_init_math(struct cx *cx);

struct cx *cx_deinit(struct cx *cx);

void cx_add_separators(struct cx *cx, const char *cs);
bool cx_is_separator(struct cx *cx, char c);

struct cx_type *_cx_add_type(struct cx *cx, const char *id, ...);
struct cx_type *cx_get_type(struct cx *cx, const char *id, bool silent);

struct cx_macro *cx_add_macro(struct cx *cx, const char *id, cx_macro_parse_t imp);
struct cx_macro *cx_get_macro(struct cx *cx, const char *id, bool silent);

struct cx_func_imp *_cx_add_func(struct cx *cx,
				 const char *id,
				 int nargs,
				 struct cx_func_arg *args);

bool cx_add_mixl_func(struct cx *cx,
		      const char *id,
		      const char *args,
		      const char *body);

struct cx_func *cx_get_func(struct cx *cx, const char *id, bool silent);

struct cx_scope *cx_scope(struct cx *cx, size_t i);
void cx_push_scope(struct cx *cx, struct cx_scope *scope);
struct cx_scope *cx_pop_scope(struct cx *cx, bool silent);
struct cx_scope *cx_begin(struct cx *cx, bool child);
void cx_end(struct cx *cx);

#endif
