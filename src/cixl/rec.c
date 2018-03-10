#include <stdlib.h>

#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/rec.h"
#include "cixl/scope.h"
#include "cixl/file.h"

static void new_imp(struct cx_box *out) {
  out->as_ptr = cx_rec_new(cx_baseof(out->type, struct cx_rec_type, imp));
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  struct cx *cx = x->type->lib->cx;
  struct cx_scope *s = cx_scope(cx, 0);
  cx_copy(cx_push(s), x);
  cx_copy(cx_push(s), y);
  if (!cx_funcall(cx, "=")) { return false; }
  return cx_test(cx_pop(s, false))->as_bool;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  return cx_cmp_ptr(&x->as_ptr, &y->as_ptr);
}

static bool ok_imp(struct cx_box *v) {
  struct cx *cx = v->type->lib->cx;
  struct cx_scope *s = cx_scope(cx, 0);
  cx_copy(cx_push(s), v);
  if (!cx_funcall(cx, "?")) { return false; }
  return cx_test(cx_pop(s, false))->as_bool;
}

static void copy_imp(struct cx_box *dst, const struct cx_box *src) {
  dst->as_ptr = cx_rec_ref(src->as_ptr);
}

static void clone_imp(struct cx_box *dst, struct cx_box *src) {
  struct cx_rec
    *src_rec = src->as_ptr,
    *dst_rec = cx_rec_new(cx_baseof(src->type, struct cx_rec_type, imp));
  
  dst->as_ptr = dst_rec;

  cx_do_env(&src_rec->fields, sv) {
    struct cx_box *dv = cx_env_put(&dst_rec->fields, sv->id);
    cx_clone(dv, &sv->value);
  }
}

static void write_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "%s new", v->type->id);
  struct cx_rec *r = v->as_ptr;
  
  cx_do_env(&r->fields, fv) {
    fprintf(out, " %% `%s ", fv->id.id);
    cx_write(&fv->value, out);
    fputs(" put", out);
  }
}

static void dump_imp(struct cx_box *v, FILE *out) {
  struct cx_rec *r = v->as_ptr;
  fprintf(out, "%s(", v->type->id);
  char sep = 0;
  
  cx_do_env(&r->fields, v) {
    if (sep) { fputc(sep, out); }
    fprintf(out, "(%s ", v->id.id);
    cx_dump(&v->value, out);
    fputc(')', out);
    sep = ' ';
  }

  fprintf(out, ")r%d", r->nrefs);
}

static void print_imp(struct cx_box *v, FILE *out) {
  struct cx *cx = v->type->lib->cx;
  struct cx_scope *s = cx_scope(cx, 0);
  cx_box_init(cx_push(s), cx->wfile_type)->as_file =
    cx_file_new(cx, fileno(out), NULL, out);
  cx_copy(cx_push(s), v);
  cx_funcall(cx, "print");
}

static void deinit_imp(struct cx_box *v) {
  cx_rec_deref(v->as_ptr);
}

static void *type_deinit_imp(struct cx_type *t) {
  struct cx_rec_type *rt = cx_baseof(t, struct cx_rec_type, imp);
  cx_set_deinit(&rt->fields);
  return rt;
}

struct cx_rec_type *cx_rec_type_init(struct cx_rec_type *type,
				     struct cx_lib *lib,
				     const char *id) {
  cx_type_init(&type->imp, lib, id);
  cx_derive(&type->imp, lib->cx->rec_type);

  type->imp.new = new_imp;
  type->imp.equid = equid_imp;
  type->imp.eqval = eqval_imp;
  type->imp.cmp = cmp_imp;
  type->imp.ok = ok_imp;
  type->imp.copy = copy_imp;
  type->imp.clone = clone_imp;
  type->imp.write = write_imp;
  type->imp.dump = dump_imp;
  type->imp.print = print_imp;
  type->imp.deinit = deinit_imp;

  type->imp.type_deinit = type_deinit_imp;

  cx_set_init(&type->fields, sizeof(struct cx_field), cx_cmp_sym);
  type->fields.key_offs = offsetof(struct cx_field, id);
  return type;
}

struct cx_rec_type *cx_rec_type_reinit(struct cx_rec_type *type) {
  cx_type_reinit(&type->imp);
  cx_derive(&type->imp, type->imp.lib->cx->rec_type);
  cx_set_clear(&type->fields);
  return type;
}

struct cx_rec_type *cx_rec_type_new(struct cx_lib *lib, const char *id) {
  return cx_rec_type_init(malloc(sizeof(struct cx_rec_type)), lib, id);
}

void cx_derive_rec(struct cx_rec_type *child, struct cx_type *parent) {
  struct cx *cx = child->imp.lib->cx;
  cx_derive(&child->imp, parent);

  if (cx_is(parent, cx->rec_type)) {
    struct cx_rec_type *rparent = cx_baseof(parent, struct cx_rec_type, imp);

    cx_do_set(&rparent->fields, struct cx_field, f) {
      cx_add_field(child, f->id, f->type, true);
    }
  }
}

bool cx_add_field(struct cx_rec_type *type,
		  struct cx_sym fid,
		  struct cx_type *ftype,
		  bool silent) {
  struct cx_field *f = cx_set_get(&type->fields, &fid);

  if (f) {
    if (!silent) {
      struct cx *cx = type->imp.lib->cx;
      
      cx_error(cx, cx->row, cx->col,
	       "Field already exists in %s: %s",
	       type->imp.id, fid.id);
    }
    
    return false;
  }

  f = cx_set_insert(&type->fields, &fid);
  f->id = fid;
  f->type = ftype;
  return true;
}

struct cx_rec *cx_rec_new(struct cx_rec_type *type) {
  struct cx *cx = type->imp.lib->cx;
  struct cx_rec *rec = cx_malloc(&cx->rec_alloc);
  rec->type = type;
  cx_env_init(&rec->fields, &cx->var_alloc);
  rec->nrefs = 1;
  return rec;
}

struct cx_rec *cx_rec_ref(struct cx_rec *rec) {
  rec->nrefs++;
  return rec;
}

void cx_rec_deref(struct cx_rec *rec) {
  cx_test(rec->nrefs);
  rec->nrefs--;
  
  if (!rec->nrefs) {
    cx_env_deinit(&rec->fields);
    cx_free(&rec->type->imp.lib->cx->rec_alloc, rec);
  }
}

struct cx_box *cx_rec_get(struct cx_rec *rec, struct cx_sym fid) {
  struct cx_var *v = cx_env_get(&rec->fields, fid);
  return v ? &v->value : NULL;
}

struct cx_box *cx_rec_put(struct cx_rec *rec, struct cx_sym fid) {
  struct cx_var *v = cx_env_get(&rec->fields, fid);

  if (v) {
    cx_box_deinit(&v->value);
    return &v->value;
  }
  
  return cx_env_put(&rec->fields, fid);
}
