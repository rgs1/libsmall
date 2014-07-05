/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

/*
 * A very simple queue implementation
 */

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <assert.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <small/queue.h>
#include <small/util.h>


struct queue {
  void **ptrs;
  int head;
  int tail;
  int count;
  int size;
  pthread_mutex_t lock;
  pthread_cond_t cond;
  void *user_data;
};


SMALL_EXPORT void queue_init(queue *q)
{
  if (pthread_mutex_init(&q->lock, 0)) {
    error(EXIT_SYSTEM_CALL, "Failed to init mutex");
  }
}

SMALL_EXPORT queue *queue_new(int size)
{
  queue *q = safe_alloc(sizeof(queue));
  q->ptrs = safe_alloc(sizeof(void *) * size);
  q->size = size;
  queue_init(q);
  return q;
}

SMALL_EXPORT void queue_destroy(queue *q)
{
  assert(q);
  assert(q->ptrs);
  free(q->ptrs);
  free(q);
}

SMALL_EXPORT int queue_add(queue *q, void *item)
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
SMALL_EXPORT int queue_empty(queue *q)
{
  return q->count == 0;
}

/* blocks until there's an element to remove */
SMALL_EXPORT void *queue_remove(queue *q)
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

SMALL_EXPORT int queue_count(queue *q)
{
  return q->count;
}


SMALL_EXPORT void queue_set_user_data(queue *q, void *data)
{
  if (q->user_data)
    info("overwriting user_data");

  q->user_data = data;
}

SMALL_EXPORT void *queue_get_user_data(queue *q)
{
  return q->user_data;
}
