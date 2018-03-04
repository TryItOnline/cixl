#include <string.h>

#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/guid.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/lib.h"
#include "cixl/lib/guid.h"

static bool guid_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box s = *cx_test(cx_pop(scope, false));
  bool ok = false;
  cx_guid_t id;
  
  if (!cx_guid_parse(s.as_str->data, id)) {
    cx_error(cx, cx->row, cx->col, "Failed parsing guid: '%s'", s.as_str->data);
    goto exit;
  }

  memcpy(cx_box_init(cx_push(scope), scope->cx->guid_type)->as_guid,
	 id,
	 sizeof(cx_guid_t));
  
  ok = true;
 exit:
  cx_box_deinit(&s);
  return ok;
}

static bool str_imp(struct cx_scope *scope) {
  struct cx_box id = *cx_test(cx_pop(scope, false));
  char s[CX_GUID_LEN];
  cx_box_init(cx_push(scope),
	      scope->cx->str_type)->as_str = cx_str_new(cx_guid_str(id.as_guid, s));
  return true;
}

cx_lib(cx_init_guid, "cx/guid", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");
    cx_use(cx, "cx/str");

    cx->guid_type = cx_init_guid_type(lib);

    cx_add_cfunc(lib, "guid",
		 cx_args(cx_arg("s", cx->str_type)),
		 cx_args(cx_arg(NULL, cx->guid_type)),
		 guid_imp);
    
    cx_add_cfunc(lib, "str",
		 cx_args(cx_arg("id", cx->guid_type)),
		 cx_args(cx_arg(NULL, cx->str_type)),
		 str_imp);
  })
