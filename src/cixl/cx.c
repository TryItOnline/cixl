#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cixl/args.h"
#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/env.h"
#include "cixl/error.h"
#include "cixl/eval.h"
#include "cixl/op.h"
#include "cixl/scan.h"
#include "cixl/scope.h"
#include "cixl/timer.h"
#include "cixl/types/bin.h"
#include "cixl/types/bool.h"
#include "cixl/types/char.h"
#include "cixl/types/file.h"
#include "cixl/types/func.h"
#include "cixl/types/guid.h"
#include "cixl/types/int.h"
#include "cixl/types/iter.h"
#include "cixl/types/lambda.h"
#include "cixl/types/nil.h"
#include "cixl/types/pair.h"
#include "cixl/types/rat.h"
#include "cixl/types/rec.h"
#include "cixl/types/ref.h"
#include "cixl/types/str.h"
#include "cixl/types/sym.h"
#include "cixl/types/table.h"
#include "cixl/types/time.h"
#include "cixl/types/vect.h"
#include "cixl/util.h"

static const void *get_type_id(const void *value) {
  struct cx_type *const *type = value;
  return &(*type)->id;
}

static const void *get_macro_id(const void *value) {
  struct cx_macro *const *macro = value;
  return &(*macro)->id;
}

static const void *get_func_id(const void *value) {
  struct cx_func *const *func = value;
  return &(*func)->id;
}

static char *get_full_path(struct cx *cx, const char *path) {
  char *full_path = (path[0] == '/' || !cx->load_paths.count)
    ? strdup(path)
    : cx_fmt("%s%s", *(char **)cx_vec_peek(&cx->load_paths, 0), path);

  char dir[strlen(path)+1];
  cx_get_dir(path, dir, sizeof(dir));
  
  if (dir[0] == '/' || !cx->load_paths.count) {
    *(char **)cx_vec_push(&cx->load_paths) = strdup(*dir ? dir : "./");
  } else {
    char *prev = *(char **)cx_vec_peek(&cx->load_paths, 0);
    *(char **)cx_vec_push(&cx->load_paths) = cx_fmt("%s%s", prev, dir);
  }

  return full_path;
}

static ssize_t include_eval(struct cx_macro_eval *eval,
			struct cx_bin *bin,
			size_t tok_idx,
			struct cx *cx) {
  if (!cx_compile(cx, cx_vec_start(&eval->toks), cx_vec_end(&eval->toks), bin)) {
    cx_error(cx, cx->row, cx->col, "Failed compiling include");
    return -1;
  }

  return tok_idx+1;
}

static bool include_parse(struct cx *cx, FILE *in, struct cx_vec *out) {
  int row = cx->row, col = cx->col;
  bool ok = false;
  
  struct cx_vec fns;
  cx_vec_init(&fns, sizeof(struct cx_tok));
  
  if (!cx_parse_end(cx, in, &fns, true)) {
    if (!cx->errors.count) { cx_error(cx, row, col, "Missing include end"); }
    goto exit1;
  }

  struct cx_macro_eval *eval = cx_macro_eval_new(include_eval);

  cx_do_vec(&fns, struct cx_tok, t) {
    if (t->type != CX_TLITERAL()) {
      cx_error(cx, t->row, t->col, "Invalid include token: %s", t->type->id);
      goto exit2;
    }

    if (t->as_box.type != cx->str_type) {
      cx_error(cx, t->row, t->col,
	       "Invalid filename: %s", t->as_box.type->id);
      goto exit2;
    }

    char *full_path = get_full_path(cx, t->as_box.as_str->data);
    bool ok = cx_load_toks(cx, full_path, &eval->toks);
    free(full_path);
    free(*(char **)cx_vec_pop(&cx->load_paths));
    if (!ok) { goto exit2; }
  }
  
  cx_tok_init(cx_vec_push(out), CX_TMACRO(), row, col)->as_ptr = eval;
  ok = true;
  goto exit1;
 exit2:
  cx_macro_eval_deref(eval);
 exit1: {
    cx_do_vec(&fns, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&fns);
    return ok;
  }
}
  
static bool call_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  bool ok = cx_call(&v, scope);
  cx_box_deinit(&v);
  return ok;
}

static bool new_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_type *t = cx_test(cx_pop(scope, false))->as_ptr;

  if (!t->new) {
    cx_error(cx, cx->row, cx->col, "%s does not implement new", t->id);
    return false;
  }
  
  struct cx_box *v = cx_push(scope);
  v->type = t;
  t->new(v);
  return true;
}

static bool clock_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_timer_t timer;
  cx_timer_reset(&timer);
  bool ok = cx_call(&v, scope);
  cx_box_deinit(&v);
  cx_box_init(cx_push(scope), scope->cx->int_type)->as_int = cx_timer_ns(&timer);
  return ok;
}

static bool check_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  struct cx *cx = scope->cx;
  bool ok = true;
  
  if (!cx_ok(&v)) {
    cx_error(cx, cx->row, cx->col, "Check failed");
    ok = false;
  }

  cx_box_deinit(&v);
  return ok;
}

static bool fail_imp(struct cx_scope *scope) {
  struct cx_box m = *cx_test(cx_pop(scope, false));
  struct cx *cx = scope->cx;
  cx_error(cx, cx->row, cx->col, m.as_str->data);
  cx_box_deinit(&m);
  return false;
}

static bool safe_imp(struct cx_scope *scope) {
  scope->safe = true;
  return true;
}

static bool unsafe_imp(struct cx_scope *scope) {
  scope->safe = false;
  return true;
}

struct cx *cx_init(struct cx *cx) {
  cx->next_sym_tag = cx->next_type_tag = 0;
  cx->bin = NULL;
  cx->pc = 0;
  cx->stop = false;
  cx->row = cx->col = -1;
  
  cx_malloc_init(&cx->lambda_alloc, CX_SLAB_SIZE, sizeof(struct cx_lambda));
  cx_malloc_init(&cx->pair_alloc, CX_SLAB_SIZE, sizeof(struct cx_pair));
  cx_malloc_init(&cx->rec_alloc, CX_SLAB_SIZE, sizeof(struct cx_rec));
  cx_malloc_init(&cx->ref_alloc, CX_SLAB_SIZE, sizeof(struct cx_ref));
  cx_malloc_init(&cx->scope_alloc, CX_SLAB_SIZE, sizeof(struct cx_scope));
  cx_malloc_init(&cx->table_alloc, CX_SLAB_SIZE, sizeof(struct cx_table));
  cx_malloc_init(&cx->var_alloc, CX_SLAB_SIZE, sizeof(struct cx_var));
  cx_malloc_init(&cx->vect_alloc, CX_SLAB_SIZE, sizeof(struct cx_vect));

  cx_set_init(&cx->separators, sizeof(char), cx_cmp_char);
  cx_add_separators(cx, " \t\n;,.|_?!()[]{}");

  cx_set_init(&cx->syms, sizeof(struct cx_sym), cx_cmp_cstr);
  cx->syms.key_offs = offsetof(struct cx_sym, id);

  cx_set_init(&cx->types, sizeof(struct cx_type *), cx_cmp_cstr);
  cx->types.key = get_type_id;
  
  cx_set_init(&cx->macros, sizeof(struct cx_macro *), cx_cmp_cstr);
  cx->macros.key = get_macro_id;

  cx_set_init(&cx->funcs, sizeof(struct cx_func *), cx_cmp_cstr);
  cx->funcs.key = get_func_id;

  cx_env_init(&cx->consts, &cx->var_alloc);
  
  cx_vec_init(&cx->load_paths, sizeof(char *));
  cx_vec_init(&cx->scopes, sizeof(struct cx_scope *));
  cx_vec_init(&cx->calls, sizeof(struct cx_call));
  cx_vec_init(&cx->errors, sizeof(struct cx_error));
  
  cx->opt_type = cx_add_type(cx, "Opt");
  cx->opt_type->trait = true;

  cx->any_type = cx_add_type(cx, "A", cx->opt_type);
  cx->any_type->trait = true;

  cx->cmp_type = cx_add_type(cx, "Cmp", cx->any_type);
  cx->cmp_type->trait = true;  

  cx->seq_type = cx_add_type(cx, "Seq", cx->any_type);
  cx->seq_type->trait = true;

  cx->num_type = cx_add_type(cx, "Num", cx->cmp_type);
  cx->num_type->trait = true;
  
  cx->rec_type = cx_add_type(cx, "Rec", cx->cmp_type);
  cx->rec_type->trait = true;

  cx->nil_type = cx_init_nil_type(cx);
  cx->meta_type = cx_init_meta_type(cx);
  
  cx->pair_type = cx_init_pair_type(cx);
  cx->iter_type = cx_init_iter_type(cx);
  cx->int_type = cx_init_int_type(cx);
  cx->bool_type = cx_init_bool_type(cx);
  cx->rat_type = cx_init_rat_type(cx);
  cx->char_type = cx_init_char_type(cx);
  cx->str_type = cx_init_str_type(cx);
  cx->sym_type = cx_init_sym_type(cx);
  cx->time_type = cx_init_time_type(cx);
  cx->guid_type = cx_init_guid_type(cx);
  cx->vect_type = cx_init_vect_type(cx);
  cx->table_type = cx_init_table_type(cx);
  cx->bin_type = cx_init_bin_type(cx);
  cx->func_type = cx_init_func_type(cx);
  cx->fimp_type = cx_init_fimp_type(cx);
  cx->lambda_type = cx_init_lambda_type(cx);
  cx->ref_type = NULL;
  
  cx->file_type = cx_init_file_type(cx, "File");
  cx->rfile_type = cx_init_file_type(cx, "RFile", cx->file_type, cx->seq_type);
  cx->rfile_type->iter = cx_file_iter;

  cx->wfile_type = cx_init_file_type(cx, "WFile", cx->file_type);
  cx->rwfile_type = cx_init_file_type(cx, "RWFile", cx->rfile_type, cx->wfile_type);
  cx->rwfile_type->iter = cx_file_iter;
  
  cx_add_macro(cx, "include:", include_parse);

  cx_add_cfunc(cx, "call", cx_args(cx_arg("act", cx->any_type)), cx_args(), call_imp);

  cx_add_cfunc(cx, "new",
	       cx_args(cx_arg("t", cx->meta_type)),
	       cx_args(cx_arg(NULL, cx->any_type)),
	       new_imp);

  cx_add_cfunc(cx, "clock",
	       cx_args(cx_arg("act", cx->any_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       clock_imp);
  
  cx_add_cfunc(cx, "check",
	       cx_args(cx_arg("v", cx->opt_type)), cx_args(),
	       check_imp);
  
  cx_add_cfunc(cx, "fail",
	       cx_args(cx_arg("msg", cx->str_type)), cx_args(),
	       fail_imp);

  cx_add_cfunc(cx, "safe", cx_args(), cx_args(), safe_imp);
  cx_add_cfunc(cx, "unsafe", cx_args(), cx_args(), unsafe_imp);

  cx->scope = NULL;
  cx->main = cx_begin(cx, NULL);
  srand((ptrdiff_t)cx + clock());

  return cx;
}

struct cx *cx_deinit(struct cx *cx) {
  cx_set_deinit(&cx->separators);

  cx_do_vec(&cx->errors, struct cx_error, e) { cx_error_deinit(e); }
  cx_vec_deinit(&cx->errors);

  cx_do_vec(&cx->calls, struct cx_call, c) { cx_call_deinit(c); }
  cx_vec_deinit(&cx->calls);

  cx_do_vec(&cx->scopes, struct cx_scope *, s) { cx_scope_deref(*s); }
  cx_vec_deinit(&cx->scopes);

  cx_do_vec(&cx->load_paths, char *, p) { free(*p); }
  cx_vec_deinit(&cx->load_paths);

  cx_env_deinit(&cx->consts);

  cx_do_set(&cx->macros, struct cx_macro *, m) { free(cx_macro_deinit(*m)); }
  cx_set_deinit(&cx->macros);

  cx_do_set(&cx->funcs, struct cx_func *, f) { free(cx_func_deinit(*f)); }
  cx_set_deinit(&cx->funcs);

  cx_do_set(&cx->types, struct cx_type *, t) { free(cx_type_deinit(*t)); }
  cx_set_deinit(&cx->types);

  cx_do_set(&cx->syms, struct cx_sym, s) { cx_sym_deinit(s); }
  cx_set_deinit(&cx->syms);

  cx_malloc_deinit(&cx->lambda_alloc);
  cx_malloc_deinit(&cx->pair_alloc);
  cx_malloc_deinit(&cx->rec_alloc);
  cx_malloc_deinit(&cx->ref_alloc);
  cx_malloc_deinit(&cx->scope_alloc);
  cx_malloc_deinit(&cx->table_alloc);
  cx_malloc_deinit(&cx->var_alloc);
  cx_malloc_deinit(&cx->vect_alloc);

  return cx;
}

void cx_add_separators(struct cx *cx, const char *cs) {
  for (const char *c = cs; *c; c++) {
    *(char *)cx_test(cx_set_insert(&cx->separators, c)) = *c;
  }
}

bool cx_is_separator(struct cx *cx, char c) {
  return cx_set_get(&cx->separators, &c);
}

struct cx_type *_cx_add_type(struct cx *cx, const char *id, ...) {
  va_list parents;
  va_start(parents, id);				
  struct cx_type *t = cx_vadd_type(cx, id, parents);
  va_end(parents);					
  return t;
}
 
struct cx_type *cx_vadd_type(struct cx *cx, const char *id, va_list parents) {
  struct cx_type **t = cx_test(cx_set_insert(&cx->types, &id));

  if (!t) {
    cx_error(cx, cx->row, cx->col, "Duplicate type: '%s'", id);
    return NULL;
  }
  
  *t = cx_type_init(malloc(sizeof(struct cx_type)), cx, id);
    
  struct cx_type *pt = NULL;
  while ((pt = va_arg(parents, struct cx_type *))) { cx_derive(*t, pt); }
  return *t;
}

struct cx_rec_type *cx_add_rec_type(struct cx *cx, const char *id) {
  struct cx_type **found = cx_set_get(&cx->types, &id);
  if (found) { return NULL; }
  
  if (found) {
    struct cx_rec_type *t = cx_baseof(*found, struct cx_rec_type, imp);
    cx_rec_type_reinit(t);
    return t;
  }
  
  struct cx_rec_type *t = cx_rec_type_new(cx, id);
  *(struct cx_type **)cx_test(cx_set_insert(&cx->types, &id)) = &t->imp;
  return t;
}

struct cx_type *cx_get_type(struct cx *cx, const char *id, bool silent) {
  struct cx_type **t = cx_set_get(&cx->types, &id);

  if (!t && !silent) {
    cx_error(cx, cx->row, cx->col, "Unknown type: '%s'", id);
  }

  return t ? *t : NULL;
}

struct cx_fimp *cx_add_func(struct cx *cx,
			    const char *id,
			    int nargs, struct cx_arg *args,
			    int nrets, struct cx_arg *rets) {
  struct cx_func **f = cx_set_get(&cx->funcs, &id);

  if (f) {
    if ((*f)->nargs != nargs) {
      cx_error(cx,
	       cx->row, cx->col,
	       "Wrong number of args for func '%s': %d/%d",
	       id, nargs, (*f)->nargs);
    }
  } else {
    f = cx_set_insert(&cx->funcs, &id);
    *f = cx_func_init(malloc(sizeof(struct cx_func)), cx, id, nargs);
  }
  
  return cx_add_fimp(*f, nargs, args, nrets, rets);
}

struct cx_fimp *cx_add_cfunc(struct cx *cx,
			     const char *id,
			     int nargs, struct cx_arg *args,
			     int nrets, struct cx_arg *rets,
			     cx_fimp_ptr_t ptr) {
  struct cx_fimp *imp = cx_add_func(cx, id, nargs, args, nrets, rets);
  imp->ptr = ptr;
  return imp;
}

struct cx_fimp *cx_add_cxfunc(struct cx *cx,
			    const char *id,
			    int nargs, struct cx_arg *args,
			    int nrets, struct cx_arg *rets,
			    const char *body) {
  struct cx_fimp *imp = cx_add_func(cx, id, nargs, args, nrets, rets);
  cx_test(cx_parse_str(cx, body, &imp->toks));
  return imp;
}

struct cx_func *cx_get_func(struct cx *cx, const char *id, bool silent) {
  struct cx_func **f = cx_set_get(&cx->funcs, &id);

  if (!f && !silent) {
    cx_error(cx, cx->row, cx->col, "Unknown func: '%s'", id);
  }

  return f ? *f : NULL;
}

struct cx_macro *cx_add_macro(struct cx *cx, const char *id, cx_macro_parse_t imp) {
  struct cx_macro **m = cx_test(cx_set_insert(&cx->macros, &id));

  if (!m) {
    cx_error(cx, cx->row, cx->col, "Duplicate macro: '%s'", id);
    return NULL;
  }

  *m = cx_macro_init(malloc(sizeof(struct cx_macro)), id, imp); 
  return *m;
}

struct cx_macro *cx_get_macro(struct cx *cx, const char *id, bool silent) {
  struct cx_macro **m = cx_set_get(&cx->macros, &id);

  if (!m && !silent) {
    cx_error(cx, cx->row, cx->col, "Unknown macro: '%s'", id);
  }
  
  return m ? *m : NULL;
}

struct cx_box *cx_get_const(struct cx *cx, struct cx_sym id, bool silent) {
  struct cx_box *v = cx_env_get(&cx->consts, id);

  if (!v) {
    if (!silent) { cx_error(cx, cx->row, cx->col, "Unknown const: '%s'", id); }
    return NULL;
  }

  return v;
}

struct cx_box *cx_set_const(struct cx *cx, struct cx_sym id, bool force) {
  struct cx_box *v = cx_env_get(&cx->consts, id);

  if (v) {
    if (!force) {
      cx_error(cx, cx->row, cx->col, "Attempt to rebind const: '%s'", id);
      return NULL;
    }
      
    cx_box_deinit(v);
  } else {
    v = cx_env_put(&cx->consts, id);
  }

  return v;
}

struct cx_sym cx_sym(struct cx *cx, const char *id) {
  struct cx_sym *s = cx_set_get(&cx->syms, &id);
  return s ? *s : *cx_sym_init(cx_set_insert(&cx->syms, &id), id, cx->next_sym_tag++);
}

struct cx_scope *cx_scope(struct cx *cx, size_t i) {
  cx_test(cx->scopes.count > i);
  return *(cx->scope-i);
}

void cx_push_scope(struct cx *cx, struct cx_scope *scope) {
  cx->scope = cx_vec_push(&cx->scopes);
  *cx->scope = cx_scope_ref(scope);
}

struct cx_scope *cx_pop_scope(struct cx *cx, bool silent) {
  if (cx->scopes.count == 1) {
    if (!silent) { cx_error(cx, cx->row, cx->col, "No open scopes"); }
    return NULL;
  }
  
  struct cx_scope *s = *cx->scope;
  cx->scope--;
  cx_vec_pop(&cx->scopes);

  if (s->stack.count) {
    struct cx_box *v = cx_vec_pop(&s->stack);
    *cx_push(cx_scope(cx, 0)) = *v;   
  }

  cx_scope_deref(s);
  return s;
}

struct cx_scope *cx_begin(struct cx *cx, struct cx_scope *parent) {
  struct cx_scope *s = cx_scope_new(cx, parent);
  cx_push_scope(cx, s);
  return s;
}

void cx_end(struct cx *cx) {
  cx_pop_scope(cx, false);
}

bool cx_funcall(struct cx *cx, const char *id) {
  struct cx_func *func = cx_get_func(cx, id, false);
  if (!func) { return false; }
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_fimp *imp = cx_func_match(func, s, 0);
  
  if (!imp) {
    cx_error(cx, cx->row, cx->col, "Func not applicable: %s", func->id);
    return false;
  }
  
  return cx_fimp_call(imp, s);
}

bool cx_load_toks(struct cx *cx, const char *path, struct cx_vec *out) {
  FILE *f = fopen(path , "r");
  
  if (!f) {
    cx_error(cx, cx->row, cx->col, "Failed opening file '%s': %d", path, errno);
    return false;
  }

  char c = fgetc(f);

  if (c == '#') {
    c = fgetc(f);

    if (c == '!') {
      while (fgetc(f) != '\n');
    } else {
      ungetc(c, f);
      ungetc('#', f);
    }
  } else {
    ungetc(c, f);
  }
  
  bool ok = cx_parse(cx, f, out);
  fclose(f);
  return ok;
}

bool cx_load(struct cx *cx, const char *path, struct cx_bin *bin) {
  bool ok = false;

  char *full_path = get_full_path(cx, path);
  struct cx_vec toks;
  cx_vec_init(&toks, sizeof(struct cx_tok));

  if (!cx_load_toks(cx, full_path, &toks)) { goto exit1; }

  if (!toks.count) {
    ok = true;
    goto exit1;
  }

  if (!cx_compile(cx, cx_vec_start(&toks), cx_vec_end(&toks), bin)) { goto exit1; }
  ok = true;
 exit1: {
    free(*(char **)cx_vec_pop(&cx->load_paths));
    cx_do_vec(&toks, struct cx_tok, t) { cx_tok_deinit(t); }
    cx_vec_deinit(&toks);
    free(full_path);
    return ok;
  }
}

void cx_dump_errors(struct cx *cx, FILE *out) {
  cx_do_vec(&cx->errors, struct cx_error, e) {
    fprintf(out, "Error in row %d, col %d:\n%s\n", e->row, e->col, e->msg);
    cx_vect_dump(&e->stack, out);
    fputs("\n\n", out);
    cx_error_deinit(e);
  }

  cx_vec_clear(&cx->errors);
}
