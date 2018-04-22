#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "cixl/arg.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/lib.h"
#include "cixl/lib/term.h"
#include "cixl/scope.h"
#include "cixl/str.h"

static bool ask_imp(struct cx_call *call) {
  struct cx_box *p = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  fputs(p->as_str->data, stdout);
  char *line = NULL;
  size_t len = 0;
  if (!cx_get_line(&line, &len, stdin)) { return false; }
  cx_box_init(cx_push(s), s->cx->str_type)->as_str = cx_str_new(line, strlen(line));
  free(line);
  return true;
}

static bool screen_size_imp(struct cx_call *call) {
  struct cx_scope *s = call->scope;
  struct winsize w;
  
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    cx_error(s->cx, s->cx->row, s->cx->col, "Failed getting screen size: %d", errno);
    return false;
  }

  cx_box_init(cx_push(s), s->cx->int_type)->as_int = w.ws_col;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = w.ws_row;
  return true;
}

static bool raw_mode_imp(struct cx_call *call) {
  struct cx_scope *s = call->scope;
  static struct termios tio;

  if (tcgetattr(STDIN_FILENO, &tio) == -1) {
    cx_error(s->cx, s->cx->row, s->cx->col, "Failed getting attribute: %d", errno);
    return false;
  }
  
  tio.c_lflag &= ~(ICANON | ECHO);

  if (tcsetattr(STDIN_FILENO, TCSANOW, &tio) == -1) {
    cx_error(s->cx, s->cx->row, s->cx->col, "Failed setting attribute: %d", errno);
    return false;
  }
  
  return true;
}

static bool normal_mode_imp(struct cx_call *call) {
  struct cx_scope *s = call->scope;
  static struct termios tio;

  if (tcgetattr(STDIN_FILENO, &tio) == -1) {
    cx_error(s->cx, s->cx->row, s->cx->col, "Failed getting attribute: %d", errno);
    return false;
  }
  
  tio.c_lflag |= ICANON | ECHO;

  if (tcsetattr(STDIN_FILENO, TCSANOW, &tio) == -1) {
    cx_error(s->cx, s->cx->row, s->cx->col, "Failed setting attribute: %d", errno);
    return false;
  }
  
  return true;
}

static bool ctrl_char_imp(struct cx_call *call) {
  struct cx_box *c = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  int cc = cx_ctrl_char(c->as_int);

  if (cc) {
    cx_box_init(cx_push(s), s->cx->int_type)->as_int = cc;
  } else {
    cx_box_init(cx_push(s), s->cx->nil_type);
  }
  
  return true;
}

static bool move_to_imp(struct cx_call *call) {
  struct cx_box
    *x = cx_test(cx_call_arg(call, 0)),
    *y = cx_test(cx_call_arg(call, 1));
  
  struct cx_scope *s = call->scope;

  char *data = cx_fmt(CX_CSI_ESC "%d;%dH", x->as_int, y->as_int);
  cx_box_init(cx_push(s), s->cx->str_type)->as_str = cx_str_new(data, -1);
  free(data);
  return true;
}

static bool set_bg_imp(struct cx_call *call) {
  struct cx_rgb *c = &cx_test(cx_call_arg(call, 0))->as_rgb;
  struct cx_scope *s = call->scope;

  char *data = cx_fmt(CX_CSI_ESC "48;2;%d;%d;%dm", c->r, c->g, c->b);
  cx_box_init(cx_push(s), s->cx->str_type)->as_str = cx_str_new(data, -1);
  free(data);
  return true;
}

static bool set_fg_imp(struct cx_call *call) {
  struct cx_rgb *c = &cx_test(cx_call_arg(call, 0))->as_rgb;
  struct cx_scope *s = call->scope;

  char *data = cx_fmt(CX_CSI_ESC "38;2;%d;%d;%dm", c->r, c->g, c->b);
  cx_box_init(cx_push(s), s->cx->str_type)->as_str = cx_str_new(data, -1);
  free(data);
  return true;
}

cx_lib(cx_init_term, "cx/io/term") {    
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Str") ||
      !cx_use(cx, "cx/gfx", "RGB") ||
      !cx_use(cx, "cx/io", "#error", "#in", "#out", "print")) {
    return false;
  }

  cx_box_init(cx_put_const(lib, cx_sym(cx, "key-esc"), false),
	      cx->int_type)->as_int = 27;
  cx_box_init(cx_put_const(lib, cx_sym(cx, "key-space"), false),
	      cx->int_type)->as_int = 32;
  cx_box_init(cx_put_const(lib, cx_sym(cx, "key-back"), false),
	      cx->int_type)->as_int = 127;

  cx_box_init(cx_put_const(lib, cx_sym(cx, "clear-screen"), false),
	      cx->str_type)->as_str = cx_str_new(CX_CSI_ESC "2J", -1);

  cx_box_init(cx_put_const(lib, cx_sym(cx, "hide-cursor"), false),
	      cx->str_type)->as_str = cx_str_new(CX_CSI_ESC "?25l", -1);
  
  cx_box_init(cx_put_const(lib, cx_sym(cx, "show-cursor"), false),
	      cx->str_type)->as_str = cx_str_new(CX_CSI_ESC "?25h", -1);

  cx_box_init(cx_put_const(lib, cx_sym(cx, "reset-style"), false),
	      cx->str_type)->as_str = cx_str_new(CX_CSI_ESC "0m", -1);

  cx_add_cxfunc(lib, "say",
		cx_args(cx_arg("v", cx->any_type)), cx_args(),
		"#out $v print\n"
		"#out @@n print");

  cx_add_cxfunc(lib, "yelp",
		cx_args(cx_arg("v", cx->any_type)), cx_args(),
		"#error $v print\n"
		"#error @@n print");

  cx_add_cfunc(lib, "ask",
	       cx_args(cx_arg("prompt", cx->str_type)), cx_args(),
	       ask_imp);

  cx_add_cfunc(lib, "screen-size",
	       cx_args(),
	       cx_args(cx_arg(NULL, cx->int_type), cx_arg(NULL, cx->int_type)),
	       screen_size_imp);

  cx_add_cfunc(lib, "raw-mode",
	       cx_args(),
	       cx_args(),
	       raw_mode_imp);

  cx_add_cfunc(lib, "normal-mode",
	       cx_args(),
	       cx_args(),
	       normal_mode_imp);

  cx_add_cfunc(lib, "ctrl-char",
	       cx_args(cx_arg("c", cx->int_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->opt_type, cx->int_type))),
	       ctrl_char_imp);

  cx_add_cfunc(lib, "move-to",
	       cx_args(cx_arg("x", cx->int_type), cx_arg("y", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->str_type)),
	       move_to_imp);  

  cx_add_cfunc(lib, "set-bg",
	       cx_args(cx_arg("c", cx->rgb_type)),
	       cx_args(cx_arg(NULL, cx->str_type)),
	       set_bg_imp);  

  cx_add_cfunc(lib, "set-fg",
	       cx_args(cx_arg("c", cx->rgb_type)),
	       cx_args(cx_arg(NULL, cx->str_type)),
	       set_fg_imp);  

  return true;
}
