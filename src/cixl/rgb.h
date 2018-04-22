#ifndef CX_RGB_H
#define CX_RGB_H

struct cx;
struct cx_type;

struct cx_rgb {
  int r, g, b;
};

struct cx_rgb *cx_rgb_init(struct cx_rgb *c,
			   int r, int g, int b);
			   
struct cx_type *cx_init_rgb_type(struct cx_lib *lib);

#endif
