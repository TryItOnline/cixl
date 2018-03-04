#include "cixl/cx.h"
#include "cixl/file.h"
#include "cixl/lib.h"
#include "cixl/lib/net.h"

cx_lib(cx_init_net, "cx/net", { 
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");
    cx_use(cx, "cx/net/types");
  })

cx_lib(cx_init_net_types, "cx/net/types", {
    struct cx *cx = lib->cx;
    cx_use(cx, "cx/abc");
    cx_use(cx, "cx/io/types");
    cx->socket_type = cx_init_file_type(lib, "Socket", cx->rwfile_type);
  })
