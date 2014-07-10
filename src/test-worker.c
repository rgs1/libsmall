/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <small/queue.h>
#include <small/worker.h>
#include <small/util.h>

static void *do_work(void *data)
{
  worker *w = (worker *)data;
  queue *q = (queue *)worker_get_data(w);
  info("doing some work...");
  queue_add(q, strdup("work was done"));
  info("done doing work...");
  return NULL;
}

static void test_basic(void)
{
  char *item;
  worker *w;
  queue *q = queue_new(1);

  w = worker_create(&do_work, q);

  /* wait for queue to be filled */
  info("waiting for queue to be filled...");
  item = (char *)queue_remove(q);
  info("grabbed element from the queue...");
  assert(strcmp(item, "work was done") == 0);
  assert(queue_count(q) == 0);
  free(item);

  queue_destroy(q);
  worker_destroy(w);
}

int main(int argc, char **argv)
{
  run_test("basic", &test_basic);

  return 0;
}
