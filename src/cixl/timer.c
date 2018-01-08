#include <stdio.h>
#include "cixl/timer.h"

void cx_timer_reset(cx_timer_t *timer) {
  clock_gettime(CLOCK_MONOTONIC, timer);
}

int64_t cx_timer_ns(cx_timer_t *timer) {
  cx_timer_t end;
  cx_timer_reset(&end);

  return
    (end.tv_sec - timer->tv_sec) * 1000000000 +
    (end.tv_nsec - timer->tv_nsec);
}
