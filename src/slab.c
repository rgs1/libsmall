/* simple, not for prod, slab implementation */

#include <assert.h>
#include <stdlib.h>

#include <small/slab.h>
#include <small/util.h>


SMALL_EXPORT void slab_init(slab_t s)
{
  if (pthread_mutex_init(&s->lock, 0)) {
    error(EXIT_SYSTEM_CALL, "Failed to init mutex");
  }
}

slab_t slab_new(int size)
{
  slab_t s = safe_alloc(sizeof(slab));
  s->mem = safe_alloc(size);
  s->size = size;
  return s;
}

SMALL_EXPORT void slab_destroy(slab_t s)
{
  assert(s->mem);
  free(s->mem);
  free(s);
}

SMALL_EXPORT void *slab_get_mem(slab_t s)
{
  return s->mem;
}

SMALL_EXPORT void * slab_get_cur(slab_t s)
{
  return (void *)((char *)s->mem + s->position);
}

SMALL_EXPORT int slab_get_size(slab_t s)
{
  return s->size;
}

SMALL_EXPORT void slab_update_position(slab_t s, int bytes)
{
  int new_pos = s->position + bytes;

  assert(new_pos <= s->size);
  s->position = new_pos;
}

SMALL_EXPORT int slab_get_position(slab_t s)
{
  return s->position;
}

SMALL_EXPORT int slab_eof(slab_t s)
{
  return s->position == s->size;
}
