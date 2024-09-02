#ifndef TIMER_H_
#define TIMER_H_

#include <time.h>

typedef struct {
  struct timespec start_time, stop_time;
  double elaped;
  int running;
} Timer;

Timer *timer_init();
void timer_free(Timer *t);

void timer_start(Timer *t);
double timer_stop(Timer *t);
void timer_continue(Timer *t);

double timer_elapsed(Timer *t);

#endif // !TIMER_H_
