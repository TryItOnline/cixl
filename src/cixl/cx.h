#ifndef CX_H
#define CX_H

#include "cixl/call.h"
#include "cixl/env.h"
#include "cixl/fimp.h"
#include "cixl/lib.h"
#include "cixl/malloc.h"
#include "cixl/parse.h"
#include "cixl/set.h"
#include "cixl/type.h"

#define CX_VERSION "0.9.7"
#define CX_SLAB_SIZE 20				  
#define CX_MAX_CALLS 64

struct cx_arg;
struct cx_catch;
struct cx_scope;
struct cx_sym;

struct cx {
  struct cx_set separators;

  struct cx_malloc
    buf_alloc,
    file_alloc,
    lambda_alloc,
    pair_alloc,
    rec_alloc, ref_alloc,
    scope_alloc, stack_alloc, stack_items_alloc,
    table_alloc,
    var_alloc;

  struct cx_vec types, macros, funcs, fimps;

  struct cx_vec links, inits;
  
  struct cx_set lib_lookup;
  struct cx_vec libs;
  struct cx_lib *lobby, **lib;

  struct cx_type *any_type,
    *bin_type, *bool_type, *buf_type,
    *char_type, *cmp_type, *color_type,
    *error_type,
    *file_type, *fimp_type, *float_type, *func_type,
    *int_type, *iter_type,
    *lambda_type, *lib_type,
    *meta_type,
    *nil_type, *num_type,
    *opt_type,
    *pair_type, *poll_type, *proc_type,
    *rec_type, *ref_type, *rfile_type, *rgb_type, *rwfile_type,
    *seq_type, *stack_type, *str_type, *sym_type,
    *table_type, *tcp_client_type, *tcp_server_type, *time_type,
    *wfile_type;

  size_t next_sym_tag, next_type_tag;
  struct cx_set syms;
  
  struct cx_vec load_paths;
  
  struct cx_vec scopes;
  struct cx_scope *root_scope, **scope;

  struct cx_call calls[CX_MAX_CALLS];
  unsigned int ncalls;
  
  struct cx_bin *bin;
  size_t pc;
  
  int row, col;
  struct cx_vec errors;
};

struct cx *cx_init(struct cx *cx);
struct cx *cx_deinit(struct cx *cx);

void cx_init_libs(struct cx *cx);

void cx_add_separators(struct cx *cx, const char *cs);
bool cx_is_separator(struct cx *cx, char c);

struct cx_lib *cx_add_lib(struct cx *cx, const char *id);
struct cx_lib *cx_get_lib(struct cx *cx, const char *id, bool silent);

void cx_push_lib(struct cx *cx, struct cx_lib *lib);
struct cx_lib *cx_pop_lib(struct cx *cx);

struct cx_sym cx_sym(struct cx *cx, const char *id);
struct cx_sym cx_gsym(struct cx *cx, const char *prefix);

struct cx_scope *cx_scope(struct cx *cx, size_t i);
void cx_push_scope(struct cx *cx, struct cx_scope *scope);
struct cx_scope *cx_pop_scope(struct cx *cx, bool silent);
struct cx_scope *cx_begin(struct cx *cx, struct cx_scope *parent);
void cx_end(struct cx *cx);

bool cx_funcall(struct cx *cx, const char *id);

struct cx_call *cx_push_call(struct cx *cx,
			     int row, int col,
			     struct cx_fimp *fimp,
			     struct cx_scope *scope);

bool cx_pop_call(struct cx *cx);
struct cx_call *cx_peek_call(struct cx *cx);

char *cx_get_path(struct cx *cx, const char *path);
bool cx_load_toks(struct cx *cx, const char *path, struct cx_vec *out);
bool cx_load(struct cx *cx, const char *path, struct cx_bin *bin);
bool cx_dlinit(struct cx *cx, const char *id);

void cx_dump_errors(struct cx *cx, FILE *out);

#endif
