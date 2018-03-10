#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/file.h"
#include "cixl/lib.h"
#include "cixl/lib/net.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static bool connect_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    port = *cx_test(cx_pop(scope, false)),
    host = *cx_test(cx_pop(scope, false));

  int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  bool ok = false;
  
  if (fd == -1) {
    cx_error(cx, cx->row, cx->col, "Failed creating socket: %d", errno);
    goto exit;
  }
  
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port.as_int);
  addr.sin_addr.s_addr = inet_addr(host.as_str->data);

  if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1 &&
      errno != EINPROGRESS) {
    cx_box_init(cx_push(scope), scope->cx->nil_type);
  } else {
    struct cx_file *f = cx_file_new(fd, "r+", NULL);
    cx_box_init(cx_push(scope), scope->cx->tcp_client_type)->as_file = f;
  }

  ok = true;
 exit:
  if (!ok) { close(fd); }
  cx_box_deinit(&host);
  return ok;
}

cx_lib(cx_init_net, "cx/net") { 
  struct cx *cx = lib->cx;

  if (!cx_use(cx, "cx/abc", "Opt") ||
      !cx_use(cx, "cx/io", "RWFile")) {
    return false;
  }

  cx->tcp_client_type = cx_init_file_type(lib, "TCPClient", cx->rwfile_type);
  
  cx_add_cfunc(lib, "tcp-connect",
	       cx_args(cx_arg("host", cx->str_type), cx_arg("port", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       connect_imp);

  return true;
}
