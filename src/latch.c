/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

/***
    a simple countdown latch
***/

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include <small/latch.h>
#include <small/util.h>

#include "util-internal.h"


struct latch {
  int count;
  pthread_mutex_t lock;
  pthread_cond_t cond;
};


SMALL_EXPORT
void latch_init(latch *l)
{
  INIT_LOCK(l);
}

SMALL_EXPORT
latch *latch_new(int count)
{
  assert(count >= 0);
  latch *l = safe_alloc(sizeof(latch));
  l->count = count;
  latch_init(l);
  return l;
}

SMALL_EXPORT
void latch_destroy(latch *l)
{
  assert(l);
  free(l);
}

SMALL_EXPORT
void latch_up(latch *l)
{
  LOCK(l);
  l->count++;
  UNLOCK(l);
}

SMALL_EXPORT
void latch_add(latch *l, int add)
{
  LOCK(l);
  l->count += add;
  UNLOCK(l);
}

SMALL_EXPORT
void latch_down(latch *l)
{
  LOCK(l);
  if (l->count > 0) {
    l->count--;
    if (l->count == 0)
      pthread_cond_broadcast(&l->cond);
  }
  UNLOCK(l);
}

SMALL_EXPORT
void latch_wait(latch *l)
{
  LOCK(l);
  while (l->count > 0)
    pthread_cond_wait(&l->cond, &l->lock);
  UNLOCK(l);
}

SMALL_EXPORT
int latch_count(latch *l)
{
  int count;
  LOCK(l);
  count = l->count;
  UNLOCK(l);
  return count;
}
