#ifndef CX_LIB_IO_H
#define CX_LIB_IO_H

struct cx;
struct cx_lib;

struct cx_lib *cx_init_io(struct cx *cx);
struct cx_lib *cx_init_io_types(struct cx *cx);

#endif
