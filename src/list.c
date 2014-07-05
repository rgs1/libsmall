/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

/*
 * a simple thread-safe list
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <small/list.h>
#include <small/pool.h>
#include <small/util.h>


struct list_item {
  void *value;
  list_item *next;
};

struct  list {
  list_item *head;
  list_item *tail;
  int count;
  int size;
  pool_t pool;
  pthread_mutex_t lock;
  pthread_cond_t cond;
};


static void * list_remove_if_matches(list *,
                                     int (*)(int, void *, void *),
                                     void *);


SMALL_EXPORT void list_init(list *l)
{
  if (pthread_mutex_init(&l->lock, 0)) {
    error(EXIT_SYSTEM_CALL, "Failed to init mutex");
  }
}

SMALL_EXPORT list *list_new(int size)
{
  list *l = safe_alloc(sizeof(list));
  l->head = l->tail = NULL;
  l->pool = pool_new(size * sizeof(list_item), sizeof(list_item));
  l->size = size;
  list_init(l);
  return l;
}

SMALL_EXPORT list_item *list_head(list *l)
{
  assert(l);
  return l->head;
}

SMALL_EXPORT void list_resize(list *l, int new_size)
{
  assert(new_size > l->size);

  pool_resize(l->pool, sizeof(list_item) * new_size);
  l->size = new_size;
}

SMALL_EXPORT void list_destroy(list *l)
{
  assert(l);
  assert(l->pool);
  pool_destroy(l->pool);
  free(l);
}

SMALL_EXPORT void *list_item_value(list_item *item)
{
  return item ? item->value : NULL;
}

SMALL_EXPORT list_item *list_item_next(list_item *item)
{
  return item ? item->next : NULL;
}

static list_item *get_free_item(list *l)
{
  list_item *item;

  assert(l->count < l->size);

  item = (list_item *)pool_get(l->pool);
  l->count++;

  return item;
}

SMALL_EXPORT void * list_prepend(list *l, void *value)
{
  list_item *item = NULL;

  LOCK(l);

  if (l->count == l->size)
    goto out;

  item = get_free_item(l);
  item->value = value;
  item->next = l->head;
  l->head = item;

  if (l->count == 1)
    l->tail = item;

out:
  UNLOCK(l);
  return item;
}

static void * do_list_append(list *l, void *value)
{
  list_item *item = NULL;

  if (l->count == l->size)
    return NULL;

  item = get_free_item(l);
  item->value = value;
  item->next = NULL;

  if (l->tail)
    l->tail->next = item;

  l->tail = item;

  if (l->count == 1)
    l->head = item;

  return item;
}

SMALL_EXPORT void * list_append(list *l, void *value)
{
  list_item *item = NULL;

  LOCK(l);

  item = do_list_append(l, value);

  UNLOCK(l);

  return item;
}

static void * item_value_transform(list_item *item)
{
  return item->value;
}

SMALL_EXPORT void list_concat(list *left, list *right)
{
  list_concat_with_transform(left, right, item_value_transform);
}

SMALL_EXPORT void list_concat_with_transform(list *left,
                                list *right,
                                void *(*transform)(list_item *))
{
  list_item *item = NULL;

  LOCK(right);
  LOCK(left);

  list_for_each_item(item, right) {
    do_list_append(left, transform(item));
  }

  UNLOCK(left);
  UNLOCK(right);
}

SMALL_EXPORT void * list_get(list *l, int pos)
{
  list_item *item = NULL;
  int i = 0;

  assert(pos < l->count);

  LOCK(l);

  list_for_each_item(item, l) {
    if (i++ == pos)
      goto out;
  }

out:
  UNLOCK(l);
  return item ? item->value : NULL;
}

static int match_by_value(int pos, void *value, void *user_data)
{
  return value == user_data;
}

SMALL_EXPORT void * list_remove_by_value(list *l, void *value)
{
  return list_remove_if_matches(l, match_by_value, value);
}

static int match_by_pos(int pos, void *value, void *user_data)
{
  return pos == (int)(long)user_data;
}

SMALL_EXPORT void * list_remove_by_pos(list *l, int pos)
{
  return list_remove_if_matches(l, match_by_pos, (void *)(long)pos);
}

/* TODO: move value comparison to a matcher func */
SMALL_EXPORT int list_contains(list *l, void *value)
{
  int ret = 0;
  list_item *item;

  LOCK(l);

  list_for_each_item(item, l) {
    if (item->value == value) {
      ret = 1;
      break;
    }
  }

  UNLOCK(l);

  return ret;
}

static void * list_remove_if_matches(list *l,
                                     int (*matcher)(int, void *, void *),
                                     void *user_data)
{
  list_item *prev = l->head;
  list_item *item;
  list_item *found = NULL;
  void *rv = NULL;
  int i = 0;

  LOCK(l);

  assert(l->count > 0);

  list_for_each_item(item, l) {
    if (matcher(i, item->value, user_data)) {
      found = item;
      rv = item->value;
      goto out;
    }
    prev = item;
    i++;
  }

out:
  if (found) {
    if (l->head == found) {
      l->head = found->next;
      if (l->count == 1)
        l->tail = NULL;
    } else if (l->tail == found) {
      l->tail = prev;
      l->tail->next = NULL;
    } else {
      prev->next = found->next;
    }

    pool_put(l->pool, found);
    l->count--;
  }
  UNLOCK(l);

  return rv;
}

SMALL_EXPORT int list_count(list *l)
{
  return l->count;
}

SMALL_EXPORT int list_full(list *l)
{
  return l->count == l->size;
}

SMALL_EXPORT void list_set_user_data(list *l, void *data)
{

}

SMALL_EXPORT void * list_get_user_data(list *l)
{
  return NULL;
}
