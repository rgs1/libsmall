/*
 * A very simple queue drop-in implementation for writing
 * quick throw away tests/prototypes.
 *
 * To test:
 *   gcc -DRUN_TESTS -Wall -lpthread queue.c -o queue
 */

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


SMALL_EXPORT void queue_init(queue_t q)
{
  if (pthread_mutex_init(&q->lock, 0)) {
    error(EXIT_SYSTEM_CALL, "Failed to init mutex");
  }
}

SMALL_EXPORT queue_t queue_new(int size)
{
  queue_t q = safe_alloc(sizeof(queue));
  q->ptrs = safe_alloc(sizeof(void *) * size);
  q->size = size;
  queue_init(q);
  return q;
}

SMALL_EXPORT void queue_destroy(queue_t q)
{
  assert(q);
  assert(q->ptrs);
  free(q->ptrs);
  free(q);
}

SMALL_EXPORT int queue_add(queue_t q, void *item)
{
  int rv = 1;

  pthread_mutex_lock(&q->lock);

  /* queue full?*/
  if (q->count == q->size) {
    warn("Can't add item, queue is full");
    rv = 0;
    goto out;
  }

  if (q->tail >= q->size) {
    q->tail = 0;
  }


  q->ptrs[q->tail] = item;
  q->tail++;
  q->count++;

  pthread_cond_broadcast(&q->cond);

out:
  pthread_mutex_unlock(&q->lock);

  return rv;
}

/*
 * you need to lock the queue to call this
 * returns 1 if empty, 0 otherwise
 */
SMALL_EXPORT int queue_empty(queue_t q)
{
  return q->count == 0;
}

/* blocks until there's an element to remove */
SMALL_EXPORT void *queue_remove(queue_t q)
{
  void *item = NULL;

  pthread_mutex_lock(&q->lock);

  while (queue_empty(q)) {
    pthread_cond_wait(&q->cond, &q->lock);
  }

  if (q->head >= q->size)
    q->head = 0;

  item = q->ptrs[q->head];
  q->head++;
  q->count--;

  pthread_mutex_unlock(&q->lock);

  return item;
}

SMALL_EXPORT int queue_count(queue_t q)
{
  return q->count;
}


SMALL_EXPORT void queue_set_user_data(queue_t q, void *data)
{
  if (q->user_data)
    info("overwriting user_data");

  q->user_data = data;
}

SMALL_EXPORT void *queue_get_user_data(queue_t q)
{
  return q->user_data;
}
