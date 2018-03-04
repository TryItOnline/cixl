#ifndef CX_LIB_TIME_H
#define CX_LIB_TIME_H

#include <stdint.h>

#define CX_USEC 1000L
#define CX_MSEC (1000*CX_USEC)
#define CX_SEC (1000*CX_MSEC)
#define CX_MIN (60*CX_SEC)
#define CX_HOUR (60*CX_MIN)
#define CX_DAY (24*CX_HOUR)

struct cx;
struct cx_lib;

struct cx_time {
  int32_t months;
  int64_t ns;
};

struct cx_time *cx_time_init(struct cx_time *time, int32_t months, int64_t ns);

struct cx_lib *cx_init_time(struct cx *cx);
struct cx_lib *cx_init_time_types(struct cx *cx);

#endif
