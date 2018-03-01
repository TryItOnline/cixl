#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cixl/arg.h"
#include "cixl/bin.h"
#include "cixl/box.h"
#include "cixl/bool.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/env.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/int.h"
#include "cixl/lambda.h"
#include "cixl/lib.h"
#include "cixl/libs/bin.h"
#include "cixl/libs/cond.h"
#include "cixl/libs/guid.h"
#include "cixl/libs/error.h"
#include "cixl/libs/func.h"
#include "cixl/libs/io.h"
#include "cixl/libs/iter.h"
#include "cixl/libs/math.h"
#include "cixl/libs/meta.h"
#include "cixl/libs/net.h"
#include "cixl/libs/pair.h"
#include "cixl/libs/rec.h"
#include "cixl/libs/ref.h"
#include "cixl/libs/stack.h"
#include "cixl/libs/str.h"
#include "cixl/libs/sym.h"
#include "cixl/libs/table.h"
#include "cixl/libs/time.h"
#include "cixl/libs/type.h"
#include "cixl/libs/var.h"
#include "cixl/nil.h"
#include "cixl/op.h"
#include "cixl/rec.h"
#include "cixl/scope.h"
#include "cixl/stack.h"
#include "cixl/str.h"
#include "cixl/timer.h"
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

static bool call_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  bool ok = cx_call(&v, scope);
  cx_box_deinit(&v);
  return ok;
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

static bool safe_imp(struct cx_scope *scope) {
  scope->safe = true;
  return true;
}

static bool unsafe_imp(struct cx_scope *scope) {
  scope->safe = false;
  return true;
}

static cx_lib(init_world, "cx", {
    return
      cx_use(cx, "cx/bin", false) &&
      cx_use(cx, "cx/cond", false) &&
      cx_use(cx, "cx/error", false) &&
      cx_use(cx, "cx/func", false) &&
      cx_use(cx, "cx/guid", false) &&
      cx_use(cx, "cx/io", false) &&
      cx_use(cx, "cx/iter", false) &&
      cx_use(cx, "cx/math", false) &&
      cx_use(cx, "cx/net", false) &&
      cx_use(cx, "cx/pair", false) &&
      cx_use(cx, "cx/rec", false) &&
      cx_use(cx, "cx/rec/io", false) &&
      cx_use(cx, "cx/ref", false) &&
      cx_use(cx, "cx/stack", false) &&
      cx_use(cx, "cx/stack/ops", false) &&
      cx_use(cx, "cx/str", false) &&
      cx_use(cx, "cx/sym", false) &&
      cx_use(cx, "cx/table", false) &&
      cx_use(cx, "cx/time", false) &&
      cx_use(cx, "cx/type", false) &&
      cx_use(cx, "cx/var", false);
  })

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
  cx_malloc_init(&cx->stack_alloc, CX_SLAB_SIZE, sizeof(struct cx_stack));

  cx_set_init(&cx->separators, sizeof(char), cx_cmp_char);
  cx_add_separators(cx, " \t\n;,.|_?!()[]{}");

  cx_set_init(&cx->syms, sizeof(struct cx_sym), cx_cmp_cstr);
  cx->syms.key_offs = offsetof(struct cx_sym, id);

  cx_set_init(&cx->libs, sizeof(struct cx_lib), cx_cmp_sym);
  cx->types.key_offs = offsetof(struct cx_lib, id);

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

  cx->bin_type =
    cx->char_type =
    cx->file_type =
    cx->fimp_type =
    cx->func_type =
    cx->guid_type =
    cx->iter_type =
    cx->lambda_type =
    cx->pair_type =
    cx->rat_type =
    cx->ref_type =
    cx->rfile_type =
    cx->rwfile_type =
    cx->socket_type =
    cx->stack_type =
    cx->str_type =
    cx->sym_type =
    cx->table_type =
    cx->time_type =
    cx->wfile_type = NULL;
  
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
  
  cx->int_type = cx_init_int_type(cx);
  cx->bool_type = cx_init_bool_type(cx);
  
  cx_add_cfunc(cx, "call", cx_args(cx_arg("act", cx->any_type)), cx_args(), call_imp);

  cx_add_cfunc(cx, "clock",
	       cx_args(cx_arg("act", cx->any_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       clock_imp);
  
  cx_add_cfunc(cx, "safe", cx_args(), cx_args(), safe_imp);
  cx_add_cfunc(cx, "unsafe", cx_args(), cx_args(), unsafe_imp);

  cx->scope = NULL;
  cx->main = cx_begin(cx, NULL);
  srand((ptrdiff_t)cx + clock());

  cx_init_bin(cx);
  cx_init_bin_types(cx);
  cx_init_cond(cx);
  cx_init_error(cx);
  cx_init_func(cx);
  cx_init_func_types(cx);
  cx_init_guid(cx);
  cx_init_guid_types(cx);
  cx_init_io(cx);
  cx_init_io_types(cx);
  cx_init_iter(cx);
  cx_init_iter_types(cx);
  cx_init_math(cx);
  cx_init_math_types(cx);
  cx_init_meta(cx);
  cx_init_net(cx);
  cx_init_net_types(cx);
  cx_init_pair(cx);
  cx_init_pair_types(cx);
  cx_init_rec(cx);
  cx_init_rec_io(cx);
  cx_init_ref(cx);
  cx_init_ref_types(cx);
  cx_init_stack(cx);
  cx_init_stack_ops(cx);
  cx_init_stack_types(cx);
  cx_init_str(cx);
  cx_init_str_types(cx);
  cx_init_sym(cx);
  cx_init_sym_types(cx);
  cx_init_table(cx);
  cx_init_table_types(cx);
  cx_init_time(cx);
  cx_init_time_types(cx);
  cx_init_type(cx);
  cx_init_var(cx);
  init_world(cx);
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

  cx_set_deinit(&cx->libs);

  cx_do_set(&cx->syms, struct cx_sym, s) { cx_sym_deinit(s); }
  cx_set_deinit(&cx->syms);

  cx_malloc_deinit(&cx->lambda_alloc);
  cx_malloc_deinit(&cx->pair_alloc);
  cx_malloc_deinit(&cx->rec_alloc);
  cx_malloc_deinit(&cx->ref_alloc);
  cx_malloc_deinit(&cx->scope_alloc);
  cx_malloc_deinit(&cx->table_alloc);
  cx_malloc_deinit(&cx->var_alloc);
  cx_malloc_deinit(&cx->stack_alloc);

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
      return NULL;
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
  if (imp) { imp->ptr = ptr; }
  return imp;
}

struct cx_fimp *cx_add_cxfunc(struct cx *cx,
			    const char *id,
			    int nargs, struct cx_arg *args,
			    int nrets, struct cx_arg *rets,
			    const char *body) {
  struct cx_fimp *imp = cx_add_func(cx, id, nargs, args, nrets, rets);
  if (imp) { cx_test(cx_parse_str(cx, body, &imp->toks)); }
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

char *cx_get_path(struct cx *cx, const char *path) {
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

  char *full_path = cx_get_path(cx, path);
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

bool cx_use(struct cx *cx, const char *id, bool silent) {
  struct cx_sym sid = cx_sym(cx, id);
  struct cx_lib *lib = cx_set_get(&cx->libs, &sid);

  if (!lib) {
    if (!silent) { cx_error(cx, cx->row, cx->col, "Lib not found: %s", id); }
    return false;
  }

  if (!lib->used) {
    lib->used = true;
    if (!lib->init(cx)) { return false; }
  }

  return true;
}

void cx_dump_errors(struct cx *cx, FILE *out) {
  cx_do_vec(&cx->errors, struct cx_error, e) {
    fprintf(out, "Error in row %d, col %d:\n%s\n", e->row, e->col, e->msg);
    cx_stack_dump(&e->stack, out);
    fputs("\n\n", out);
    cx_error_deinit(e);
  }

  cx_vec_clear(&cx->errors);
}
