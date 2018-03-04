#include "cixl/bool.h"
#include "cixl/cx.h"
#include "cixl/int.h"
#include "cixl/lib.h"
#include "cixl/nil.h"
#include "cixl/lib/abc.h"

cx_lib(cx_init_abc, "cx/abc", { 
    struct cx *cx = lib->cx;

    cx->opt_type = cx_add_type(lib, "Opt");
    cx->opt_type->trait = true;

    cx->any_type = cx_add_type(lib, "A", cx->opt_type);
    cx->any_type->trait = true;

    cx->cmp_type = cx_add_type(lib, "Cmp", cx->any_type);
    cx->cmp_type->trait = true;  

    cx->seq_type = cx_add_type(lib, "Seq", cx->any_type);
    cx->seq_type->trait = true;

    cx->num_type = cx_add_type(lib, "Num", cx->cmp_type);
    cx->num_type->trait = true;
  
    cx->rec_type = cx_add_type(lib, "Rec", cx->cmp_type);
    cx->rec_type->trait = true;

    cx->nil_type = cx_init_nil_type(lib);
    cx->meta_type = cx_init_meta_type(lib);
    cx->int_type = cx_init_int_type(lib);
    cx->bool_type = cx_init_bool_type(lib);
  })
