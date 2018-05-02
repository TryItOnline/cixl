#include "cixl/term.h"

void cx_set_bg(unsigned char r, unsigned char g, unsigned char b, FILE *out) {
  fprintf(out, CX_CSI_ESC "48;2;%d;%d;%dm", r, g, b);
}

void cx_move_right(int n, FILE *out) {
  fprintf(out, CX_CSI_ESC "%dC", n);
}

void cx_move_left(int n, FILE *out) {
  fprintf(out, CX_CSI_ESC "%dD", n);
}

void cx_move_down(int n, FILE *out) {
  fprintf(out, CX_CSI_ESC "%dB", n);
}

void cx_reset_style(FILE *out) {
  fputs(CX_CSI_ESC "0m", out);
}
