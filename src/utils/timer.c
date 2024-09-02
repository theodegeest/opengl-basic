#include "timer.h"
#include <stdlib.h>

Timer *timer_init() {
  Timer *t = malloc(sizeof(Timer));
  t->elaped = 0;
  t->running = 0;
  return t;
}

void timer_free(Timer *t) {
  free(t);
}

void timer_start(Timer *t) {
  t->running = 1;
  t->elaped = 0;
  clock_gettime(CLOCK_MONOTONIC, &t->start_time);
}

double timer_stop(Timer *t) {
  clock_gettime(CLOCK_MONOTONIC, &t->stop_time);
  t->running = 0;
  t->elaped += (t->stop_time.tv_sec - t->start_time.tv_sec) +
              (t->stop_time.tv_nsec - t->start_time.tv_nsec) / 1000000.0;
  return t->elaped;
}

void timer_continue(Timer *t) {
  t->running = 1;
  clock_gettime(CLOCK_MONOTONIC, &t->start_time);
}

double timer_elapsed(Timer *t) {
  if (t->running) {
    return (t->stop_time.tv_sec - t->start_time.tv_sec) +
           (t->stop_time.tv_nsec - t->start_time.tv_nsec) / 1000000.0;
  } else {
    return t->elaped;
  }
}
