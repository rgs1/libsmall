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
#include <small/util.h>


static void *producer(void *data)
{
  char *a = "hello";
  char *b = "goodbye";
  queue_t q = (queue_t)data;

  info("Adding a=%s", a);
  queue_add(q, a);
  sleep(2);
  info("Adding b=%s", b);
  queue_add(q, b);

  info("count = %d", queue_count(q));

  /* wait until all is consumed */
  while (!queue_empty(q))
    sleep(1);

  return NULL;
}

static void *consumer(void *data)
{
  queue_t q = (queue_t)data;

  info("removed item = %s", (char *)queue_remove(q));
  sleep(4);
  info("removed item = %s", (char *)queue_remove(q));

  info("count = %d", queue_count(q));

  return NULL;
}

static void test_basic(void)
{
  queue_t q = queue_new(2);
  pthread_t producer_tid, consumer_tid;

  pthread_create(&producer_tid, NULL, &producer, q);
  pthread_create(&consumer_tid, NULL, &consumer, q);

  pthread_join(producer_tid, NULL);
  pthread_join(consumer_tid, NULL);

  assert(queue_count(q) == 0);

  queue_destroy(q);
}

static void test_queue_full(void)
{
  char *a = "hello";
  char *b = "goodbye";
  queue_t q = queue_new(1);

  assert(queue_add(q, a));
  info("count = %d", queue_count(q));
  assert(queue_count(q) == 1);
  assert(!queue_add(q, b));
  info("count = %d", queue_count(q));
  assert(queue_count(q) == 1);

  queue_destroy(q);
}

static void test_more_than_size(void)
{
  queue_t q = queue_new(3);

  queue_add(q, NULL);
  queue_add(q, NULL);
  queue_add(q, NULL);

  info("count = %d", queue_count(q));
  assert(queue_count(q) == 3);

  queue_remove(q);
  info("count = %d", queue_count(q));
  assert(queue_count(q) == 2);

  queue_add(q, NULL);
  info("count = %d", queue_count(q));
  assert(queue_count(q) == 3);

  queue_remove(q);
  info("count = %d", queue_count(q));
  assert(queue_count(q) == 2);

  queue_remove(q);
  info("count = %d", queue_count(q));
  assert(queue_count(q) == 1);

  queue_remove(q);
  info("count = %d", queue_count(q));
  assert(queue_count(q) == 0);
}

static void test_right_value(void)
{
  int a = 10;
  int b = 20;
  int c = 30;
  int *item;
  queue_t q = queue_new(3);

  queue_add(q, &a);
  queue_add(q, &b);
  queue_add(q, &c);

  item = queue_remove(q);
  info("item = %d", *item);
  assert(*item == 10);

  queue_add(q, &a);

  item = queue_remove(q);
  info("item = %d", *item);
  assert(*item == 20);

  item = queue_remove(q);
  info("item = %d", *item);
  assert(*item == 30);

  item = queue_remove(q);
  info("item = %d", *item);
  assert(*item == 10);

  info("count = %d", queue_count(q));
  assert(queue_count(q) == 0);
}

int main(int argc, char **argv)
{
  run_test("basic", &test_basic);
  run_test("queue full", &test_queue_full);
  run_test("add/remove more than size items", &test_more_than_size);
  run_test("get back the right element", &test_right_value);

  return 0;
}
