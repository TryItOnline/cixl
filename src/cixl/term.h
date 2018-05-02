#ifndef CX_TERM_H
#define CX_TERM_H

#include <stdio.h>

#define CX_CSI_ESC "\x1b["

void cx_set_bg(unsigned char r, unsigned char g, unsigned char b, FILE *out);
void cx_move_right(int n, FILE *out);
void cx_move_left(int n, FILE *out);
void cx_move_down(int n, FILE *out);
void cx_reset_style(FILE *out);

#endif
