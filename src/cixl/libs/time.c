#include <errno.h>
#include <time.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/buf.h"
#include "cixl/error.h"
#include "cixl/libs/io.h"
#include "cixl/scope.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"
#include "cixl/types/time.h"

static bool years_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       n.as_int*12, 0);

  return true;
}

static bool months_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       n.as_int, 0);

  return true;
}

static bool days_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int*CX_DAY);

  return true;
}

static bool h_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int*CX_HOUR);

  return true;
}

static bool m_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int*CX_MIN);

  return true;
}

static bool s_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int*CX_SEC);

  return true;
}

static bool ms_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int*CX_MSEC);

  return true;
}

static bool us_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int*CX_USEC);

  return true;
}

static bool ns_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box n = *cx_test(cx_pop(scope, false));

  cx_time_init(&cx_box_init(cx_push(scope), cx->time_type)->as_time,
	       0, n.as_int);
  
  return true;
}

static bool now_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  struct tm tm;
  
  if (!localtime_r(&ts.tv_sec, &tm)) {
    cx_error(cx, cx->row, cx->col, "Failed destructuring time: %d", errno);
  }
  
  struct cx_time *t = &cx_box_init(cx_push(scope), cx->time_type)->as_time;

  cx_time_init(t,
	       (tm.tm_year+1900)*12 + tm.tm_mon,
	       (tm.tm_mday-1) * CX_DAY +
	       (tm.tm_hour) * CX_HOUR +
	       (tm.tm_min) * CX_MIN +
	       (tm.tm_sec) * CX_SEC +
	       ts.tv_nsec);
  
  return true;
}

static bool time_years_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.months / 12;
  return true;
}

static bool month_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.months % 12;
  return true;
}

static bool time_months_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.months;
  return true;
}

static bool time_days_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns / CX_DAY;
  return true;
}

static bool hour_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope),
	      cx->int_type)->as_int = (t.as_time.ns % CX_DAY) / CX_HOUR;
  return true;
}

static bool minute_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope),
	      cx->int_type)->as_int = (t.as_time.ns % CX_HOUR) / CX_MIN;
  return true;
}

static bool second_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope),
	      cx->int_type)->as_int = (t.as_time.ns % CX_MIN) / CX_SEC;
  return true;
}

static bool nsecond_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = (t.as_time.ns % CX_SEC);
  return true;
}

static bool time_h_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns / CX_HOUR;
  return true;
}

static bool time_m_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns / CX_MIN;
  return true;
}

static bool time_s_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns / CX_SEC;
  return true;
}

static bool time_ms_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns / CX_MSEC;
  return true;
}

static bool time_us_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns / CX_USEC;
  return true;
}

static bool time_ns_imp(struct cx_scope *scope) {
  struct cx *cx = scope->cx;
  struct cx_box t = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), cx->int_type)->as_int = t.as_time.ns;
  return true;
}

static bool add_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    *x = cx_test(cx_peek(scope, false));
  
  x->as_time.months += y.as_time.months;
  x->as_time.ns += y.as_time.ns;
  return true;
}

static bool sub_imp(struct cx_scope *scope) {
  struct cx_box
    y = *cx_test(cx_pop(scope, false)),
    *x = cx_test(cx_peek(scope, false));
  
  x->as_time.months -= y.as_time.months;
  x->as_time.ns -= y.as_time.ns;
  return true;
}

static bool lt_imp(struct cx_scope *scope) {
  struct cx_time
    yt = cx_test(cx_pop(scope, false))->as_time,
    xt = cx_test(cx_pop(scope, false))->as_time;
  
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool =
    xt.months < yt.months ||
    (xt.months == yt.months && xt.ns < yt.ns);
  
  return true;
}

static bool gt_imp(struct cx_scope *scope) {
  struct cx_time
    yt = cx_test(cx_pop(scope, false))->as_time,
    xt = cx_test(cx_pop(scope, false))->as_time;
  
  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool =
    xt.months > yt.months ||
    (xt.months == yt.months && xt.ns > yt.ns);
  
  return true;
}

void cx_init_time(struct cx *cx) {
  cx_add_func(cx, "years", cx_arg(cx->int_type))->ptr = years_imp;
  cx_add_func(cx, "months", cx_arg(cx->int_type))->ptr = months_imp;
  cx_add_func(cx, "days", cx_arg(cx->int_type))->ptr = days_imp;
  cx_add_func(cx, "h", cx_arg(cx->int_type))->ptr = h_imp;
  cx_add_func(cx, "m", cx_arg(cx->int_type))->ptr = m_imp;
  cx_add_func(cx, "s", cx_arg(cx->int_type))->ptr = s_imp;
  cx_add_func(cx, "ms", cx_arg(cx->int_type))->ptr = ms_imp;
  cx_add_func(cx, "us", cx_arg(cx->int_type))->ptr = us_imp;
  cx_add_func(cx, "ns", cx_arg(cx->int_type))->ptr = ns_imp;
  
  cx_add_func(cx, "now")->ptr = now_imp;
  cx_add_func(cx, "years", cx_arg(cx->time_type))->ptr = time_years_imp;
  cx_add_func(cx, "month", cx_arg(cx->time_type))->ptr = month_imp;
  cx_add_func(cx, "months", cx_arg(cx->time_type))->ptr = time_months_imp;
  cx_add_func(cx, "days", cx_arg(cx->time_type))->ptr = time_days_imp;

  cx_add_func(cx, "hour", cx_arg(cx->time_type))->ptr = hour_imp;
  cx_add_func(cx, "minute", cx_arg(cx->time_type))->ptr = minute_imp;
  cx_add_func(cx, "second", cx_arg(cx->time_type))->ptr = second_imp;
  cx_add_func(cx, "nsecond", cx_arg(cx->time_type))->ptr = nsecond_imp;
  
  cx_add_func(cx, "h", cx_arg(cx->time_type))->ptr = time_h_imp;
  cx_add_func(cx, "m", cx_arg(cx->time_type))->ptr = time_m_imp;
  cx_add_func(cx, "s", cx_arg(cx->time_type))->ptr = time_s_imp;

  cx_add_func(cx, "ms", cx_arg(cx->time_type))->ptr = time_ms_imp;
  cx_add_func(cx, "us", cx_arg(cx->time_type))->ptr = time_us_imp;
  cx_add_func(cx, "ns", cx_arg(cx->time_type))->ptr = time_ns_imp;

  
  cx_add_func(cx, "+",
	      cx_arg(cx->time_type), cx_arg(cx->time_type))->ptr = add_imp;
  cx_add_func(cx, "-",
	      cx_arg(cx->time_type), cx_arg(cx->time_type))->ptr = sub_imp;

  cx_add_func(cx, "<",
	      cx_arg(cx->time_type), cx_arg(cx->time_type))->ptr = lt_imp;
  cx_add_func(cx, ">",
	      cx_arg(cx->time_type), cx_arg(cx->time_type))->ptr = gt_imp;
}
