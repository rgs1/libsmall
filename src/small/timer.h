#ifndef _TIMER_H_
#define _TIMER_H_

typedef struct timer timer;

timer *timer_new(void);
void timer_init(timer *t);
void timer_destroy(timer *t);
void timer_start(timer *t);
void timer_stop(timer *t);
char *timer_diff(timer *t, char *out, int outlen);

#endif
