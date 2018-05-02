#ifndef CX_COLOR_H
#define CX_COLOR_H

struct cx_lib;
struct cx_type;

struct cx_color {
  unsigned int r, g, b, a;
};

struct cx_color *cx_color_init(struct cx_color *c,
			       unsigned int r, unsigned int g, unsigned int b,
			       unsigned int a);
			   
struct cx_type *cx_init_color_type(struct cx_lib *lib);

#endif
