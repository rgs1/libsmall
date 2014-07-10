/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

/* a memory pool manager */

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include <small/util.h>

#include "pool.h"
#include "slab.h"
#include "util-internal.h"


struct pool {
  int item_size;
  int size;
  slab **slabs;
  int slab_count;
  int slab_curr;
  void **free_list;
  int free_count;
  pthread_mutex_t lock;
  pthread_cond_t cond;
};


SMALL_EXPORT void pool_init(pool *p)
{
  INIT_LOCK(p);
}

static void add_slab(pool *p, int size)
{
  int index = p->slab_count++;
  size_t old = sizeof(slab *) * index;
  size_t new = sizeof(slab *) * p->slab_count;

  p->slabs = safe_realloc(p->slabs, old, new);
  p->slabs[index] = slab_new(size);

  /* update free list */
  old = sizeof(void *) * index;
  new = sizeof(void *) * (p->size / p->item_size);
  p->free_list = safe_realloc(p->free_list, old, new);
}

SMALL_EXPORT pool *pool_new(int size, int item_size)
{
  pool *p = safe_alloc(sizeof(pool));
  pool_init(p);
  p->size = size;
  p->item_size = item_size;
  add_slab(p, size);
  return p;
}

SMALL_EXPORT void pool_destroy(pool *p)
{
  int i;

  assert(p);
  assert(p->slabs);
  assert(p->free_list);

  for (i=0; i < p->slab_count; i++) {
    slab_destroy(p->slabs[i]);
  }

  free(p->slabs);
  free(p->free_list);
  free(p);
}

static slab *get_usable_slab(pool *p)
{
  slab *s = p->slabs[p->slab_curr];

  if (!slab_eof(s))
    return s;

  if (p->slab_curr + 1 < p->slab_count) {
    return p->slabs[++p->slab_curr];
  }

  return NULL;
}

SMALL_EXPORT void * pool_get(pool *p)
{
  slab *s;
  void *item = NULL;
  int last;

  LOCK(p);

  if (p->free_count > 0) {
    item = p->free_list[0];
    last = --p->free_count;
    p->free_list[0] = p->free_list[last];
  } else {
    s = get_usable_slab(p);
    if (s) {
      item = slab_get_cur(s);
      slab_update_position(s, p->item_size);
    }
  }

  UNLOCK(p);

  return item;
}

SMALL_EXPORT void pool_put(pool *p, void *item)
{
  assert(p->free_count >= 0);

  LOCK(p);
  p->free_list[p->free_count++] = item;
  UNLOCK(p);
}

SMALL_EXPORT void pool_resize(pool *p, int new_size)
{
  size_t size;

  LOCK(p);

  assert(new_size > p->size);

  size = new_size - p->size;
  p->size = new_size;
  add_slab(p, size);
  UNLOCK(p);
}
