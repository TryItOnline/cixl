#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/lib.h"
#include "cixl/mfile.h"
#include "cixl/parse.h"
#include "cixl/scope.h"
#include "cixl/type_set.h"

static struct cx_type *type_new_imp(struct cx_type *t,
				    const char *id,
				    int nargs, struct cx_type *args[]) {
  struct cx_type_set
    *ts = cx_baseof(t, struct cx_type_set, imp),
    *nts = cx_type_set_new(t->lib, id, false);

  cx_do_set(&ts->set, struct cx_type *, mt) {
    *(struct cx_type **)cx_vec_push(&nts->set.members) = *mt;
  }

  return &nts->imp;
}

static void *type_deinit_imp(struct cx_type *t) {
  struct cx_type_set *ts = cx_baseof(t, struct cx_type_set, imp);
  cx_set_deinit(&ts->parents);
  cx_set_deinit(&ts->set);
  return ts;
}

struct cx_type_set *cx_type_set_new(struct cx_lib *lib,
				    const char *id,
				    bool raw) {
  struct cx_type_set *ts = malloc(sizeof(struct cx_type_set));
  
  const char *i = strchr(id, '<');
  
  if (i && raw) {
    char tid[i-id+1];
    strncpy(tid, id, i-id);
    tid[i-id] = 0;
    cx_type_init(&ts->imp, lib, tid);
  } else {
    cx_type_init(&ts->imp, lib, id);
  }
  
  cx_set_init(&ts->parents, sizeof(struct cx_type *), cx_cmp_ptr);
  cx_set_init(&ts->set, sizeof(struct cx_type *), cx_cmp_ptr);
  
  ts->imp.type_new = type_new_imp;
  ts->imp.type_deinit = type_deinit_imp;

  if (i) {
    i++;
    
    char args[strlen(i)+1];
    strcpy(args, i);
    char *j = args;

    while (true) {
      struct cx_type *tt = cx_parse_type_arg(lib->cx, &j);
      if (!tt) { break; }
      *(struct cx_type **)cx_vec_push(&ts->imp.args) = tt;
    }
  }
  
  return ts;
}

static char *conv_id(const char *in) {
  struct cx_mfile out;
  cx_mfile_open(&out);

  for (const char *c = in; *c; c++) {
    if (isupper(*c)) {
      if (c != in) { fputc('-', out.stream); }
      fputc(tolower(*c), out.stream);
    } else {
      fputc(*c, out.stream);
    }
  }
  
  cx_mfile_close(&out);
  return out.data;
}

static bool conv_imp(struct cx_call *call) {
  struct cx_box *in = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  struct cx_box *out = cx_push(s);
  cx_copy(out, in);

  struct cx_arg *ret = cx_test(cx_vec_start(&call->fimp->rets));
  out->type = ret->type;
  return true;
}

void cx_type_define_conv(struct cx_type *t, struct cx_type *mt) {
  char *cid = conv_id(t->raw->id);
  
  cx_add_cfunc(t->lib, cid,
	       cx_args(cx_arg("in", mt)),
	       cx_args(cx_arg(NULL, t)),
	       conv_imp);
  
  free(cid);
  cid = conv_id(mt->id);
  
  cx_add_cfunc(t->lib, cid,
	       cx_args(cx_arg("in", t)),
	       cx_args(cx_arg(NULL, mt)),
	       conv_imp);
  
  free(cid);
}
