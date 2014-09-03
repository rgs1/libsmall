/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <small/timer.h>
#include <small/util.h>


static void test_basic(void)
{
  char buf[64];
  struct timespec ts = { .tv_sec = 0, .tv_nsec = 500000000 };
  timer *t = timer_new();

  info("Starting");
  timer_start(t);
  nanosleep(&ts, NULL);
  timer_stop(t);
  info("Diff: %s", timer_diff(t, buf, 64));
}


int main(int argc, char **argv)
{
  run_test("basic", &test_basic);

  return 0;
}
