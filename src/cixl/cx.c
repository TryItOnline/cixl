#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
#include "cixl/lib/abc.h"
#include "cixl/lib/bin.h"
#include "cixl/lib/cond.h"
#include "cixl/lib/const.h"
#include "cixl/lib/guid.h"
#include "cixl/lib/error.h"
#include "cixl/lib/func.h"
#include "cixl/lib/io.h"
#include "cixl/lib/iter.h"
#include "cixl/lib/math.h"
#include "cixl/lib/meta.h"
#include "cixl/lib/net.h"
#include "cixl/lib/pair.h"
#include "cixl/lib/rec.h"
#include "cixl/lib/ref.h"
#include "cixl/lib/stack.h"
#include "cixl/lib/str.h"
#include "cixl/lib/sym.h"
#include "cixl/lib/table.h"
#include "cixl/lib/time.h"
#include "cixl/lib/type.h"
#include "cixl/lib/var.h"
#include "cixl/nil.h"
#include "cixl/op.h"
#include "cixl/pair.h"
#include "cixl/poll.h"
#include "cixl/rec.h"
#include "cixl/ref.h"
#include "cixl/scope.h"
#include "cixl/stack.h"
#include "cixl/str.h"
#include "cixl/table.h"
#include "cixl/util.h"

static const void *get_lib_id(const void *value) {
  struct cx_lib *const *lib = value;
  return &(*lib)->id;
}

cx_lib(cx_init_world, "cx") {
  struct cx *cx = lib->cx;
  
  return
    cx_use(cx, "cx/abc") &&
    cx_use(cx, "cx/bin") &&
    cx_use(cx, "cx/cond") &&
    cx_use(cx, "cx/const") &&
    cx_use(cx, "cx/error") &&
    cx_use(cx, "cx/func") &&
    cx_use(cx, "cx/guid") &&
    cx_use(cx, "cx/io") &&
    cx_use(cx, "cx/iter") &&
    cx_use(cx, "cx/math") &&
    cx_use(cx, "cx/meta") &&
    cx_use(cx, "cx/net") &&
    cx_use(cx, "cx/pair") &&
    cx_use(cx, "cx/rec") &&
    cx_use(cx, "cx/ref") &&
    cx_use(cx, "cx/stack") &&
    cx_use(cx, "cx/str") &&
    cx_use(cx, "cx/sym") &&
    cx_use(cx, "cx/table") &&
    cx_use(cx, "cx/time") &&
    cx_use(cx, "cx/type") &&
    cx_use(cx, "cx/var");
}

struct cx *cx_init(struct cx *cx) {
  cx->next_sym_tag = cx->next_type_tag = 0;
  cx->poll = NULL;
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

  cx_vec_init(&cx->types, sizeof(struct cx_type *));
  cx_vec_init(&cx->macros, sizeof(struct cx_macro *));
  cx_vec_init(&cx->funcs, sizeof(struct cx_func *));
  cx_vec_init(&cx->fimps, sizeof(struct cx_fimp *));
  
  cx_set_init(&cx->lib_lookup, sizeof(struct cx_lib *), cx_cmp_sym);
  cx->lib_lookup.key = get_lib_id;
  cx_vec_init(&cx->libs, sizeof(struct cx_lib *));

  cx->lobby = cx_add_lib(cx, "lobby");
  cx_push_lib(cx, cx->lobby);
  
  cx_vec_init(&cx->load_paths, sizeof(char *));
  cx_vec_init(&cx->scopes, sizeof(struct cx_scope *));
  cx_vec_init(&cx->calls, sizeof(struct cx_call));
  cx_vec_init(&cx->errors, sizeof(struct cx_error));

  cx->any_type =
    cx->bin_type =
    cx->bool_type =
    cx->char_type =
    cx->cmp_type =
    cx->file_type =
    cx->fimp_type =
    cx->func_type =
    cx->guid_type =
    cx->int_type =
    cx->iter_type =
    cx->lambda_type =
    cx->lib_type = 
    cx->nil_type =
    cx->num_type =
    cx->meta_type =
    cx->opt_type =
    cx->pair_type =
    cx->rat_type =
    cx->rec_type =
    cx->ref_type =
    cx->rfile_type =
    cx->rwfile_type =
    cx->seq_type =
    cx->stack_type =
    cx->str_type =
    cx->sym_type =
    cx->table_type =
    cx->tcp_client_type =
    cx->time_type =
    cx->wfile_type = NULL;
      
  cx->scope = NULL;
  cx->root_scope = cx_begin(cx, NULL);
  srand((ptrdiff_t)cx + clock());

  return cx;
}

void cx_init_libs(struct cx *cx) {
  cx_init_abc(cx);
  cx_init_bin(cx);
  cx_init_cond(cx);
  cx_init_const(cx);
  cx_init_error(cx);
  cx_init_func(cx);
  cx_init_guid(cx);
  cx_init_io(cx);
  cx_init_iter(cx);
  cx_init_math(cx);
  cx_init_meta(cx);
  cx_init_net(cx);
  cx_init_pair(cx);
  cx_init_rec(cx);
  cx_init_ref(cx);
  cx_init_stack(cx);
  cx_init_str(cx);
  cx_init_sym(cx);
  cx_init_table(cx);
  cx_init_time(cx);
  cx_init_type(cx);
  cx_init_var(cx);
  cx_init_world(cx);
}

struct cx *cx_deinit(struct cx *cx) {
  cx_set_deinit(&cx->separators);

  cx_do_vec(&cx->errors, struct cx_error, e) { cx_error_deinit(e); }
  cx_vec_deinit(&cx->errors);

  if (cx->poll) { free(cx_poll_deinit(cx->poll)); }

  cx_do_vec(&cx->calls, struct cx_call, c) { cx_call_deinit(c); }
  cx_vec_deinit(&cx->calls);

  cx_do_vec(&cx->scopes, struct cx_scope *, s) { cx_scope_deref(*s); }
  cx_vec_deinit(&cx->scopes);

  cx_do_vec(&cx->load_paths, char *, p) { free(*p); }
  cx_vec_deinit(&cx->load_paths);
  
  cx_do_vec(&cx->macros, struct cx_macro *, m) { free(cx_macro_deinit(*m)); }
  cx_vec_deinit(&cx->macros);

  cx_do_vec(&cx->funcs, struct cx_func *, f) { free(cx_func_deinit(*f)); }
  cx_vec_deinit(&cx->funcs);

  cx_do_vec(&cx->fimps, struct cx_fimp *, f) { free(cx_fimp_deinit(*f)); }
  cx_vec_deinit(&cx->fimps);

  cx_do_set(&cx->lib_lookup, struct cx_lib *, l) { free(cx_lib_deinit(*l)); }
  cx_set_deinit(&cx->lib_lookup);
  cx_vec_deinit(&cx->libs);

  cx_do_vec(&cx->types, struct cx_type *, t) { free(cx_type_deinit(*t)); }
  cx_vec_deinit(&cx->types);

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

struct cx_lib *cx_add_lib(struct cx *cx, const char *id) {
  struct cx_sym sid = cx_sym(cx, id);
  struct cx_lib **lib = cx_set_get(&cx->lib_lookup, &sid);
  
  if (!lib) {
    lib = cx_set_insert(&cx->lib_lookup, &sid);
    *lib = cx_lib_init(malloc(sizeof(struct cx_lib)), cx, sid);
  }
  
  return *lib;
}

struct cx_lib *cx_get_lib(struct cx *cx, const char *id, bool silent) {
  struct cx_sym sid = cx_sym(cx, id);
  struct cx_lib **ok = cx_set_get(&cx->lib_lookup, &sid);

  if (!ok) {
    if (!silent) { cx_error(cx, cx->row, cx->col, "Lib not found: %s", id); }
    return NULL;
  }

  return *ok;
}

void cx_push_lib(struct cx *cx, struct cx_lib *lib) {
  cx->lib = cx_vec_push(&cx->libs);
  *cx->lib = lib;
}

void cx_pop_lib(struct cx *cx) {
  cx_test(cx->libs.count > 1);
  cx_vec_pop(&cx->libs);
  cx->lib--;
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
  struct cx_func *func = cx_get_func(*cx->lib, id, false);
  if (!func) { return false; }
  struct cx_scope *s = cx_scope(cx, 0);
  struct cx_fimp *imp = cx_func_match(func, s);
  
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

struct cx_poll *cx_poll(struct cx *cx) {
  if (!cx->poll) { cx->poll = cx_poll_init(malloc(sizeof(struct cx_poll))); }
  return cx->poll;
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

