#include "cixl/cx.h"
#include "cixl/lib.h"
#include "cixl/libs/net.h"
#include "cixl/types/file.h"

cx_lib(cx_init_net, "cx/net", { 
    if (!cx_use(cx, "cx/net/types", false)) { return false; }
    return true; 
  })

cx_lib(cx_init_net_types, "cx/net/types", {
    if (!cx_use(cx, "cx/io/types", false)) { return false; }
    cx->socket_type = cx_init_file_type(cx, "Socket", cx->rwfile_type);
    return true;
  })
