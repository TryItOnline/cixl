#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "cixl/arg.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/file.h"
#include "cixl/lib.h"
#include "cixl/lib/term.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/term.h"

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

static bool clear_screen_imp(struct cx_call *call) {
  struct cx_box *out = cx_test(cx_call_arg(call, 0));
  fputs(CX_CSI_ESC "2J", cx_file_ptr(out->as_file));
  return true;
}

static bool clear_screen_end_imp(struct cx_call *call) {
  struct cx_box *out = cx_test(cx_call_arg(call, 0));
  fputs(CX_CSI_ESC "0J", cx_file_ptr(out->as_file));
  return true;
}

static bool reset_style_imp(struct cx_call *call) {
  struct cx_box *out = cx_test(cx_call_arg(call, 0));
  cx_reset_style(cx_file_ptr(out->as_file));
  return true;
}

static bool reverse_colors_imp(struct cx_call *call) {
  struct cx_box *out = cx_test(cx_call_arg(call, 0));
  fputs(CX_CSI_ESC "7m", cx_file_ptr(out->as_file));
  return true;
}

static bool save_cursor_imp(struct cx_call *call) {
  struct cx_box *out = cx_test(cx_call_arg(call, 0));
  fputs(CX_CSI_ESC "s", cx_file_ptr(out->as_file));
  return true;
}

static bool restore_cursor_imp(struct cx_call *call) {
  struct cx_box *out = cx_test(cx_call_arg(call, 0));
  fputs(CX_CSI_ESC "u", cx_file_ptr(out->as_file));
  return true;
}

static bool hide_cursor_imp(struct cx_call *call) {
  struct cx_box *out = cx_test(cx_call_arg(call, 0));
  fputs(CX_CSI_ESC "?25l", cx_file_ptr(out->as_file));
  return true;
}

static bool show_cursor_imp(struct cx_call *call) {
  struct cx_box *out = cx_test(cx_call_arg(call, 0));
  fputs(CX_CSI_ESC "?25h", cx_file_ptr(out->as_file));
  return true;
}

static bool clear_row_imp(struct cx_call *call) {
  struct cx_box *out = cx_test(cx_call_arg(call, 0));
  fputs(CX_CSI_ESC "2K", cx_file_ptr(out->as_file));
  return true;
}

static bool clear_row_end_imp(struct cx_call *call) {
  struct cx_box *out = cx_test(cx_call_arg(call, 0));
  fputs(CX_CSI_ESC "0K", cx_file_ptr(out->as_file));
  return true;
}

static bool move_up_imp(struct cx_call *call) {
  struct cx_box
    *n = cx_test(cx_call_arg(call, 0)),
    *out = cx_test(cx_call_arg(call, 1));
  
  fprintf(cx_file_ptr(out->as_file), CX_CSI_ESC "%ldA", n->as_int);
  return true;
}

static bool move_down_imp(struct cx_call *call) {
  struct cx_box
    *n = cx_test(cx_call_arg(call, 0)),
    *out = cx_test(cx_call_arg(call, 1));
  cx_move_down(n->as_int, cx_file_ptr(out->as_file));
  return true;
}

static bool move_left_imp(struct cx_call *call) {
  struct cx_box
    *n = cx_test(cx_call_arg(call, 0)),
    *out = cx_test(cx_call_arg(call, 1));

  cx_move_left(n->as_int, cx_file_ptr(out->as_file));
  return true;
}

static bool move_right_imp(struct cx_call *call) {
  struct cx_box
    *n = cx_test(cx_call_arg(call, 0)),
    *out = cx_test(cx_call_arg(call, 1));
  cx_move_right(n->as_int, cx_file_ptr(out->as_file));
  return true;
}

static bool move_to_imp(struct cx_call *call) {
  FILE *out = cx_file_ptr(cx_test(cx_call_arg(call, 0))->as_file);
  struct cx_point *pos = &cx_test(cx_call_arg(call, 1))->as_point;
  fprintf(out, CX_CSI_ESC "%ld;%ldH", (int64_t)pos->y, (int64_t)pos->x);
  return true;
}

static bool set_bg_imp(struct cx_call *call) {
  struct cx_color *c = &cx_test(cx_call_arg(call, 0))->as_color;
  struct cx_file *out = cx_test(cx_call_arg(call, 1))->as_file;
  cx_set_bg(c->r, c->g, c->b, cx_file_ptr(out));
  return true;
}

static bool set_fg_imp(struct cx_call *call) {
  struct cx_color *c = &cx_test(cx_call_arg(call, 0))->as_color;
  struct cx_file *out = cx_test(cx_call_arg(call, 1))->as_file;
  fprintf(cx_file_ptr(out), CX_CSI_ESC "38;2;%d;%d;%dm", c->r, c->g, c->b);
  return true;
}

cx_lib(cx_init_term, "cx/io/term") {    
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Str") ||
      !cx_use(cx, "cx/gfx", "Color") ||
      !cx_use(cx, "cx/io", "WFile", "#error", "#in", "#out", "print") ||
      !cx_use(cx, "cx/iter", "times")) {
    return false;
  }

  cx_box_init(cx_put_const(lib, cx_sym(cx, "key-esc"), false),
	      cx->int_type)->as_int = 27;
  cx_box_init(cx_put_const(lib, cx_sym(cx, "key-space"), false),
	      cx->int_type)->as_int = 32;
  cx_box_init(cx_put_const(lib, cx_sym(cx, "key-back"), false),
	      cx->int_type)->as_int = 127;

  cx_add_cxfunc(lib, "say",
		cx_args(cx_arg("v", cx->opt_type)), cx_args(),
		"$v #out print\n"
		"@@n #out print");

  cx_add_cxfunc(lib, "yelp",
		cx_args(cx_arg("v", cx->any_type)), cx_args(),
		"$v #error print\n"
		"@@n #error print");

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

  cx_add_cfunc(lib, "clear-screen",
	       cx_args(cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       clear_screen_imp);  

  cx_add_cfunc(lib, "clear-screen-end",
	       cx_args(cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       clear_screen_end_imp);  

  cx_add_cfunc(lib, "reset-style",
	       cx_args(cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       reset_style_imp);  

  cx_add_cfunc(lib, "reverse-colors",
	       cx_args(cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       reverse_colors_imp);  
  
  cx_add_cfunc(lib, "save-cursor",
	       cx_args(cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       save_cursor_imp);  

  cx_add_cfunc(lib, "restore-cursor",
	       cx_args(cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       restore_cursor_imp);  

  cx_add_cfunc(lib, "hide-cursor",
	       cx_args(cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       hide_cursor_imp);  

  cx_add_cfunc(lib, "show-cursor",
	       cx_args(cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       show_cursor_imp);  

  cx_add_cfunc(lib, "clear-row",
	       cx_args(cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       clear_row_imp);  

  cx_add_cfunc(lib, "clear-row-end",
	       cx_args(cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       clear_row_end_imp);  

  cx_add_cfunc(lib, "move-up",
	       cx_args(cx_arg("n", cx->int_type), cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       move_up_imp);  

  cx_add_cfunc(lib, "move-down",
	       cx_args(cx_arg("n", cx->int_type), cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       move_down_imp);  

  cx_add_cfunc(lib, "move-left",
	       cx_args(cx_arg("n", cx->int_type), cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       move_left_imp);  

  cx_add_cfunc(lib, "move-right",
	       cx_args(cx_arg("n", cx->int_type), cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       move_right_imp);  

  cx_add_cfunc(lib, "move-to",
	       cx_args(cx_arg("out", cx->wfile_type), cx_arg("pos", cx->point_type)),
	       cx_args(),
	       move_to_imp);  

  cx_add_cfunc(lib, "set-bg",
	       cx_args(cx_arg("c", cx->color_type),
		       cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       set_bg_imp);  

  cx_add_cfunc(lib, "set-fg",
	       cx_args(cx_arg("c", cx->color_type),
		       cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       set_fg_imp);  

  cx_add_cxfunc(lib, "hline",
	       cx_args(cx_arg("c", cx->char_type),
		       cx_arg("n", cx->int_type),
		       cx_arg("out", cx->wfile_type)),
	       cx_args(),
	       "$n {$c #out print} times");  

  cx_add_cxfunc(lib, "vline",
		cx_args(cx_arg("c", cx->char_type),
			cx_arg("n", cx->int_type),
			cx_arg("out", cx->wfile_type)),
		cx_args(),
		"$n {\n"
		"  $c #out print\n"
		"  1 $out move-down\n"
		"  1 $out move-left\n"
		"} times");  
      
  return true;
}
