/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <small/latch.h>
#include <small/util.h>


static void *worker(void *data)
{
  latch *l = (latch *)data;

  info("Down by 1");
  latch_down(l);
  sleep(2);
  info("Down by 1");
  latch_down(l);
  info("Count = %d", latch_count(l));

  return NULL;
}

static void *watcher(void *data)
{
  latch *l = (latch *)data;

  info("Waiting");
  latch_wait(l);
  info("Done waiting");

  return NULL;
}

static void test_basic(void)
{
  latch *l = latch_new(0);
  pthread_t worker_tid, watcher_tid;

  info("Up 2 times");
  latch_up(l);
  latch_up(l);

  pthread_create(&worker_tid, NULL, &worker, l);
  pthread_create(&watcher_tid, NULL, &watcher, l);

  pthread_join(worker_tid, NULL);
  pthread_join(watcher_tid, NULL);

  assert(latch_count(l) == 0);

  latch_destroy(l);
}


int main(int argc, char **argv)
{
  run_test("basic", &test_basic);

  return 0;
}
