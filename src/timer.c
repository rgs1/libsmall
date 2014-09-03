/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

/***
    a timer of usec precision
***/

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <small/timer.h>
#include <small/util.h>

#include "util-internal.h"


struct timer {
  int running;
  struct timeval start;
  struct timeval end;
  pthread_mutex_t lock;
};


SMALL_EXPORT
void timer_init(timer *t)
{
  INIT_LOCK(t);
}

SMALL_EXPORT
timer *timer_new(void)
{
  timer *t = safe_alloc(sizeof(timer));
  timer_init(t);
  return t;
}

SMALL_EXPORT
void timer_destroy(timer *t)
{
  assert(t);
  free(t);
}

SMALL_EXPORT
void timer_start(timer *t)
{
  LOCK(t);
  if (!t->running) {
    gettimeofday(&t->start, NULL);
    t->running = 1;
  }
  UNLOCK(t);
}

SMALL_EXPORT
void timer_stop(timer *t)
{
  LOCK(t);
  if (t->running) {
    gettimeofday(&t->end, NULL);
    t->running = 0;
  }
  UNLOCK(t);
}

static void prettyp(struct timeval *tv, char *out, int len)
{
    struct tm *nowtm;
    char tmp[64];

    nowtm = localtime(&tv->tv_sec);
    strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", nowtm);
    snprintf(out, len, "%s.%06d", tmp, (int)tv->tv_usec);
}

static void to_usec(struct timeval *tv, char *out, int len)
{
    struct tm *nowtm;
    char tmp[64];

    nowtm = localtime(&tv->tv_sec);
    strftime(tmp, sizeof(tmp), "%S", nowtm);
    snprintf(out, len, "%s.%06d", tmp, (int)tv->tv_usec);
}

SMALL_EXPORT
char * timer_diff(timer *t, char *out, int outlen)
{
  LOCK(t);
  if (!t->running) {
    struct timeval diff;

    timersub(&t->end, &t->start, &diff);
    to_usec(&diff, out, outlen);
  }
  UNLOCK(t);

  return out;
}
