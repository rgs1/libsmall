/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

/* simple slab implementation */

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include <small/util.h>

#include "slab.h"


struct slab {
  void *mem;
  int position;
  int size;
  pthread_mutex_t lock;
  pthread_cond_t cond;
};



SMALL_EXPORT void slab_init(slab *s)
{
  if (pthread_mutex_init(&s->lock, 0)) {
    error(EXIT_SYSTEM_CALL, "Failed to init mutex");
  }
}

slab *slab_new(int size)
{
  slab *s = safe_alloc(sizeof(slab));
  s->mem = safe_alloc(size);
  s->size = size;
  return s;
}

SMALL_EXPORT void slab_destroy(slab *s)
{
  assert(s->mem);
  free(s->mem);
  free(s);
}

SMALL_EXPORT void *slab_get_mem(slab *s)
{
  return s->mem;
}

SMALL_EXPORT void * slab_get_cur(slab *s)
{
  return (void *)((char *)s->mem + s->position);
}

SMALL_EXPORT int slab_get_size(slab *s)
{
  return s->size;
}

SMALL_EXPORT void slab_update_position(slab *s, int bytes)
{
  int new_pos = s->position + bytes;

  assert(new_pos <= s->size);
  s->position = new_pos;
}

SMALL_EXPORT int slab_get_position(slab *s)
{
  return s->position;
}

SMALL_EXPORT int slab_eof(slab *s)
{
  return s->position == s->size;
}
