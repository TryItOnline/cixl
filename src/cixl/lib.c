#include <ctype.h>

#include "cixl/cx.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/rec.h"
#include "cixl/set.h"

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

struct cx_lib *cx_lib_init(struct cx_lib *lib,
			   struct cx *cx,
			   struct cx_sym id,
			   cx_lib_init_t init) {
  lib->cx = cx;
  lib->id = id;
  lib->emit_id = cx_emit_id("lib", id.id);
  lib->init = init;
  
  cx_set_init(&lib->types, sizeof(struct cx_type *), cx_cmp_cstr);
  lib->types.key = get_type_id;

  cx_set_init(&lib->macros, sizeof(struct cx_macro *), cx_cmp_cstr);
  lib->macros.key = get_macro_id;

  cx_set_init(&lib->funcs, sizeof(struct cx_func *), cx_cmp_cstr);
  lib->funcs.key = get_func_id;

  cx_env_init(&lib->consts, &cx->var_alloc);

  return lib;
}

struct cx_lib *cx_lib_deinit(struct cx_lib *lib) {
  cx_env_deinit(&lib->consts);
  cx_set_deinit(&lib->funcs);
  cx_set_deinit(&lib->macros);
  cx_set_deinit(&lib->types);
  free(lib->emit_id);
  return lib;
}

struct cx_type *_cx_add_type(struct cx_lib *lib, const char *id, ...) {
  va_list parents;
  va_start(parents, id);				
  struct cx_type *t = cx_vadd_type(lib, id, parents);
  va_end(parents);					
  return t;
}
 
struct cx_type *cx_vadd_type(struct cx_lib *lib, const char *id, va_list parents) {
  struct cx *cx = lib->cx;
  struct cx_type **t = cx_test(cx_set_insert(&lib->types, &id));

  if (!t) {
    cx_error(cx, cx->row, cx->col, "Duplicate type: '%s'", id);
    return NULL;
  }
  
  *t = cx_type_init(malloc(sizeof(struct cx_type)), cx, id);
  *(struct cx_type **)cx_vec_push(&cx->types) = *t;
  
  struct cx_type *pt = NULL;
  while ((pt = va_arg(parents, struct cx_type *))) { cx_derive(*t, pt); }
  return *t;
}

struct cx_rec_type *cx_add_rec_type(struct cx_lib *lib, const char *id) {
  struct cx *cx = lib->cx;
  struct cx_type **found = cx_set_get(&lib->types, &id);
  if (found) { return NULL; }
  
  if (found) {
    struct cx_rec_type *t = cx_baseof(*found, struct cx_rec_type, imp);
    cx_rec_type_reinit(t);
    return t;
  }
  
  struct cx_rec_type *t = cx_rec_type_new(cx, id);
  *(struct cx_type **)cx_vec_push(&cx->types) = &t->imp;
  *(struct cx_type **)cx_test(cx_set_insert(&lib->types, &id)) = &t->imp;
  return t;
}

struct cx_type *cx_get_type(struct cx_lib *lib, const char *id, bool silent) {
  struct cx_type **t = cx_set_get(&lib->types, &id);

  if (!t && !silent) {
    struct cx *cx = lib->cx;
    cx_error(cx, cx->row, cx->col, "Unknown type: '%s'", id);
  }

  return t ? *t : NULL;
}

struct cx_macro *cx_add_macro(struct cx_lib *lib,
			      const char *id,
			      cx_macro_parse_t imp) {
  struct cx *cx= lib->cx;
  struct cx_macro **m = cx_test(cx_set_insert(&lib->macros, &id));

  if (!m) {
    cx_error(cx, cx->row, cx->col, "Duplicate macro: '%s'", id);
    return NULL;
  }

  *m = cx_macro_init(malloc(sizeof(struct cx_macro)), id, imp); 
  *(struct cx_macro **)cx_vec_push(&cx->macros) = *m;
  return *m;
}

struct cx_macro *cx_get_macro(struct cx_lib *lib, const char *id, bool silent) {
  struct cx_macro **m = cx_set_get(&lib->macros, &id);

  if (!m && !silent) {
    struct cx *cx = lib->cx;
    cx_error(cx, cx->row, cx->col, "Unknown macro: '%s'", id);
  }
  
  return m ? *m : NULL;
}

struct cx_fimp *cx_add_func(struct cx_lib *lib,
			    const char *id,
			    int nargs, struct cx_arg *args,
			    int nrets, struct cx_arg *rets) {
  struct cx *cx = lib->cx;
  struct cx_func **f = cx_set_get(&lib->funcs, &id);

  if (f) {
    if ((*f)->nargs != nargs) {
      cx_error(cx,
	       cx->row, cx->col,
	       "Wrong number of args for func '%s': %d/%d",
	       id, nargs, (*f)->nargs);
      return NULL;
    }
  } else {
    f = cx_set_insert(&lib->funcs, &id);
    *f = cx_func_init(malloc(sizeof(struct cx_func)), lib, id, nargs);
    *(struct cx_func **)cx_vec_push(&cx->funcs) = *f;
  }
  
  return cx_add_fimp(*f, nargs, args, nrets, rets);
}

struct cx_fimp *cx_add_cfunc(struct cx_lib *lib,
			     const char *id,
			     int nargs, struct cx_arg *args,
			     int nrets, struct cx_arg *rets,
			     cx_fimp_ptr_t ptr) {
  struct cx_fimp *imp = cx_add_func(lib, id, nargs, args, nrets, rets);
  if (imp) { imp->ptr = ptr; }
  return imp;
}

struct cx_fimp *cx_add_cxfunc(struct cx_lib *lib,
			      const char *id,
			      int nargs, struct cx_arg *args,
			      int nrets, struct cx_arg *rets,
			      const char *body) {
  struct cx_fimp *imp = cx_add_func(lib, id, nargs, args, nrets, rets);
  if (imp) { cx_parse_str(lib->cx, body, &imp->toks); }
  return imp;
}

struct cx_func *cx_get_func(struct cx_lib *lib, const char *id, bool silent) {
  struct cx_func **f = cx_set_get(&lib->funcs, &id);

  if (!f && !silent) {
    struct cx *cx = lib->cx;
    cx_error(cx, cx->row, cx->col, "Unknown func: '%s'", id);
  }

  return f ? *f : NULL;
}

struct cx_box *cx_get_const(struct cx_lib *lib, struct cx_sym id, bool silent) {
  struct cx_var *v = cx_env_get(&lib->consts, id);

  if (!v) {
    if (!silent) {
      struct cx *cx = lib->cx;
      cx_error(cx, cx->row, cx->col, "Unknown const: '%s'", id.id);
    }
    
    return NULL;
  }

  return &v->value;
}

struct cx_box *cx_set_const(struct cx_lib *lib, struct cx_sym id, bool force) {
  struct cx_var *v = cx_env_get(&lib->consts, id);

  if (v) {
    if (!force) {
      struct cx *cx = lib->cx;
      cx_error(cx, cx->row, cx->col, "Attempt to rebind const: '%s'", id);
      return NULL;
    }
      
    cx_box_deinit(&v->value);
    return &v->value;
  }

  return cx_env_put(&lib->consts, id);
}

static void use_type(struct cx_type *t, struct cx_lib *dst) {
  struct cx_type **ok = cx_set_insert(&dst->types, &t->id);
  if (ok) { *ok = t; }
 }

static void use_macro(struct cx_macro *m, struct cx_lib *dst) {
  struct cx_macro **ok = cx_set_insert(&dst->macros, &m->id);
  if (ok) { *ok = m; }
}

static bool use_func(struct cx_func *f, struct cx_lib *dst) {
  struct cx_func **ok = cx_set_get(&dst->funcs, &f->id);

  if (ok) {
    if ((*ok)->nargs != f->nargs) {
      struct cx *cx = f->lib->cx;
      cx_error(cx, cx->row, cx->col, "Wrong arity (%d/%d) for func: %s/%s",
	       f->nargs, (*ok)->nargs, dst->id.id, f->id);
      return false;
    }
    
    cx_do_set(&f->imps, struct cx_fimp *, i) {
      cx_ensure_fimp(*ok, *i);
    }
  } else {
    ok = cx_set_insert(&dst->funcs, &f->id);
    *ok = f;
  }
  
  return true;
}

static void use_const(struct cx_var *v, struct cx_lib *dst) {
  struct cx_var *prev = cx_env_get(&dst->consts, v->id);
  if (!prev) { cx_copy(cx_env_put(&dst->consts, v->id), &v->value); }
}

static bool use_all(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  
  cx_do_set(&lib->types, struct cx_type *, t) { use_type(*t, *cx->lib); }
  
  cx_do_set(&lib->macros, struct cx_macro *, m) { use_macro(*m, *cx->lib); }
  
  cx_do_set(&lib->funcs, struct cx_func *, f) {
    if (!use_func(*f, *cx->lib)) { return false; }
  }

  cx_do_env(&lib->consts, v) { use_const(v, *cx->lib); }
  return true;
}

static bool use_id(struct cx_lib *lib, const char *id) {
  struct cx *cx = lib->cx;

  if (id[0] == '#') {
    struct cx_var *v = cx_env_get(&lib->consts, cx_sym(cx, id+1));

    if (v) {
      use_const(v, *cx->lib);
      return true;
    }
  } else if (isupper(id[0])) {
    struct cx_type **t = cx_set_get(&lib->types, &id);
    
    if (t) {
      use_type(*t, *cx->lib);
      return true;
    }
  } else {
    struct cx_macro **m = cx_set_get(&lib->macros, &id);

    if (m) {
      use_macro(*m, *cx->lib);
      return true;
    }
    
    struct cx_func **f = cx_set_get(&lib->funcs, &id);
    if (f) { return use_func(*f, *cx->lib); }
  }

  cx_error(cx, cx->row, cx->col, "%s/%s not found", lib->id.id, id);
  return false;
}

bool _cx_use(struct cx *cx,
	     const char *lib_id,
	     unsigned int nids, const char **ids) {
  struct cx_sym lid = cx_sym(cx, lib_id);
  struct cx_lib **ok = cx_set_get(&cx->lib_lookup, &lid);

  if (!ok) {
    cx_error(cx, cx->row, cx->col, "Lib not found: %s", lib_id);
    return false;
  }

  struct cx_lib *lib = *ok;

  if (lib->init) {
    cx_push_lib(cx, lib);
    lib->init(lib);
    lib->init = NULL;
    cx_pop_lib(cx);
  }

  if (nids) {
    for (unsigned int i = 0; i < nids; i++) {
      if (!use_id(lib, ids[i])) { return false; }
    }

    return true;
  }
  
  return use_all(lib);
}
