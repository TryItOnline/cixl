#include "cixl/cx.h"
#include "cixl/file.h"
#include "cixl/lib.h"
#include "cixl/lib/net.h"

cx_lib(cx_init_net, "cx/net") { 
  struct cx *cx = lib->cx;

  if (!cx_use(cx, "cx/io", "RWFile")) {
    return false;
  }

  cx->socket_type = cx_init_file_type(lib, "Socket", cx->rwfile_type);

  return true;
}
