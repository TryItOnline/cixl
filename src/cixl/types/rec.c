#include <stdlib.h>

#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/types/rec.h"

struct cx_field_value {
  struct cx_sym id;
  struct cx_box box;
}; 

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static bool eqval_imp(struct cx_box *x, struct cx_box *y) {
  struct cx_rec *xr = x->as_ptr, *yr = y->as_ptr;
  if (xr->values.members.count != yr->values.members.count) { return false; }
  
  for (size_t i = 0; i < xr->values.members.count; i++) {
    struct cx_field_value
      *xv = cx_vec_get(&xr->values.members, i),
      *yv = cx_vec_get(&yr->values.members, i);
    
    if (xv->id.tag != yv->id.tag || !cx_eqval(&xv->box, &yv->box)) {
      return false;
    }
  }
  
  return true;
}

static bool ok_imp(struct cx_box *v) {
  struct cx_rec *r = v->as_ptr;
  return r->values.members.count;
}

static void copy_imp(struct cx_box *dst, struct cx_box *src) {
  dst->as_ptr = cx_rec_ref(src->as_ptr);
}

static void clone_imp(struct cx_box *dst, struct cx_box *src) {
  struct cx_rec *src_rec = src->as_ptr, *dst_rec = cx_rec_new();
  dst->as_ptr = dst_rec;

  cx_do_set(&src_rec->values, struct cx_field_value, sv) {
    struct cx_field_value *dv = cx_test(cx_set_insert(&dst_rec->values, &sv->id));
    dv->id = sv->id;
    cx_clone(&dv->box, &sv->box);
  }
}

static void fprint_imp(struct cx_box *v, FILE *out) {
  struct cx_rec *r = v->as_ptr;
  fprintf(out, "%s(%p)@%d", v->type->id, r, r->nrefs);
}

static void deinit_imp(struct cx_box *v) {
  cx_rec_unref(v->as_ptr);
}

static void type_deinit_imp(struct cx_type *t) {
  struct cx_rec_type *rt = cx_baseof(t, struct cx_rec_type, imp);
  cx_set_deinit(&rt->fields);
}

struct cx_rec_type *cx_rec_type_init(struct cx_rec_type *type,
				     struct cx *cx,
				     const char *id) {
  cx_type_init(&type->imp, cx, id);

  type->imp.equid = equid_imp;
  type->imp.eqval = eqval_imp;
  type->imp.ok = ok_imp;
  type->imp.copy = copy_imp;
  type->imp.clone = clone_imp;
  type->imp.fprint = fprint_imp;
  type->imp.deinit = deinit_imp;

  type->imp.type_deinit = type_deinit_imp;

  cx_set_init(&type->fields, sizeof(struct cx_field), cx_cmp_sym);
  type->fields.key_offs = offsetof(struct cx_field, id);
  cx_derive(&type->imp, cx->rec_type);
  return type;
}

struct cx_rec_type *cx_rec_type_reinit(struct cx_rec_type *type) {
  cx_type_reinit(&type->imp);
  cx_derive(&type->imp, type->imp.cx->rec_type);
  cx_set_clear(&type->fields);
  return type;
}

struct cx_rec_type *cx_rec_type_new(struct cx *cx, const char *id) {
  return cx_rec_type_init(malloc(sizeof(struct cx_rec_type)), cx, id);
}

struct cx_rec_type *cx_rec_type_deinit(struct cx_rec_type *type) {
  cx_type_deinit(&type->imp);
  return type;
}

void cx_derive_rec(struct cx_rec_type *child, struct cx_type *parent) {
  struct cx *cx = parent->cx;
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
      struct cx *cx = type->imp.cx;
      
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

struct cx_rec *cx_rec_new() {
  struct cx_rec *rec = malloc(sizeof(struct cx_rec));
  cx_set_init(&rec->values, sizeof(struct cx_field_value), cx_cmp_sym);
  rec->nrefs = 1;
  return rec;
}

struct cx_rec *cx_rec_ref(struct cx_rec *rec) {
  rec->nrefs++;
  return rec;
}

void cx_rec_unref(struct cx_rec *rec) {
  cx_test(rec->nrefs > 0);
  rec->nrefs--;
  if (!rec->nrefs) {
    cx_do_set(&rec->values, struct cx_field_value, v) { cx_box_deinit(&v->box); }
    cx_set_deinit(&rec->values);
    free(rec);
  }
}

struct cx_box *cx_rec_get(struct cx_rec *rec, struct cx_sym fid) {
  struct cx_field_value *f = cx_set_get(&rec->values, &fid);
  return f ? &f->box : NULL;
}

void cx_rec_put(struct cx_rec *rec, struct cx_sym fid, struct cx_box *v) {
  struct cx_field_value *f = cx_set_get(&rec->values, &fid);

  if (f) {
    cx_box_deinit(&f->box);
  } else if (!f) {
    f = cx_set_insert(&rec->values, &fid);
  }
  
  f->id = fid;
  cx_copy(&f->box, v);
}
