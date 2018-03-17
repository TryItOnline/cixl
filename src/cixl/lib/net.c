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

static bool listen_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  
  struct cx_box
    backlog = *cx_test(cx_pop(scope, false)),
    port = *cx_test(cx_pop(scope, false)),
    host = *cx_test(cx_pop(scope, false));

  int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  bool ok = false;
  
  if (fd == -1) {
    cx_error(cx, cx->row, cx->col, "Failed creating socket: %d", errno);
    goto exit;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
    cx_error(cx, cx->row, cx->col, "Failed enabling socket reuse: %d", errno);
    goto exit;
  }
  
  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port.as_int);
  addr.sin_addr.s_addr = (host.type == cx->nil_type)
    ? INADDR_ANY
    : inet_addr(host.as_str->data);
  
  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    cx_error(cx, cx->row, cx->col, "Failed binding socket: %d", errno);
    goto exit;
  }

  if (listen(fd, backlog.as_int) == -1) {
    cx_error(cx, cx->row, cx->col, "Failed listening on socket: %d", errno);
    goto exit;
  }
    
  struct cx_file *f = cx_file_new(cx, fd, "r", NULL);
  cx_box_init(cx_push(scope), scope->cx->tcp_server_type)->as_file = f;
  ok = true;
 exit:
  if (!ok) { close(fd); }
  cx_box_deinit(&host);
  return ok;
}

static bool accept_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box server = *cx_test(cx_pop(scope, false));
  int fd = accept(server.as_file->fd, NULL, NULL);
  bool ok = false;
  
  if (fd == -1) {
    cx_box_init(cx_push(scope), cx->nil_type);
  } else {
    struct cx_file *f = cx_file_new(cx, fd, "r+", NULL);
    if (!cx_file_unblock(f)) { goto exit; }
    cx_box_init(cx_push(scope), cx->tcp_client_type)->as_file = f;
  }

  ok = true;
 exit:
  cx_box_deinit(&server);
  return ok;
}


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
    struct cx_file *f = cx_file_new(cx, fd, "r+", NULL);
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

  if (!cx_use(cx, "cx/abc", "Int", "Opt", "Str") ||
      !cx_use(cx, "cx/io", "RFile", "RWFile")) {
    return false;
  }

  cx->tcp_server_type = cx_init_file_type(lib, "TCPServer", cx->rfile_type);
  cx->tcp_client_type = cx_init_file_type(lib, "TCPClient", cx->rwfile_type);

  cx_add_cfunc(lib, "listen",
	       cx_args(cx_arg("host", cx->opt_type),
		       cx_arg("port", cx->int_type),
		       cx_arg("backlog", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->tcp_server_type)),
	       listen_imp);

  cx_add_cfunc(lib, "accept",
	       cx_args(cx_arg("server", cx->tcp_server_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       accept_imp);

  cx_add_cfunc(lib, "connect",
	       cx_args(cx_arg("host", cx->str_type), cx_arg("port", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->opt_type)),
	       connect_imp);

  return true;
}
