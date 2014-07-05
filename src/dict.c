/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

/*
 * a simple, fixed size & thread-safe, dictionary
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <small/dict.h>
#include <small/list.h>
#include <small/util.h>

#include "pool.h"


struct dict_key_value {
  void *key;
  void *value;
};

typedef struct dict_key_value dict_key_value;

struct dict {
  list **keys;
  pool *pool;
  int count;
  int size;
  int (*key_comparator)(void *a, void *b); /* 0 if =, -1 if a < b, 1 if a > b */
  int (*hash_func)(void *key, int size);
  pthread_mutex_t lock;
  pthread_cond_t cond;
  void *user_data;
};

#define DICT_KEY_COLLISIONS     10

#define keys_for_each_list(dict, i, list)                \
  for (i = 0, list = dict->keys[0]; i < dict->size; i++, list = dict->keys[i])


SMALL_EXPORT void dict_init(dict *d)
{
  INIT_LOCK(d);
}

static int default_key_comparator(void *a, void *b)
{
  return a == b ? 0 : (a < b ? -1 : 1);
}


/* Note:
 *  the step)_factor size usually is the number of bytes between each key
 */
static int default_hash_func(void *key, int size)
{
  return (int)(((long)key / 32) % size);
}

SMALL_EXPORT dict *dict_new(int size)
{
  int i;
  dict *d = safe_alloc(sizeof(dict));

  /* init lists */
  d->keys = (list **)safe_alloc(sizeof(list *) * size);
  for (i=0; i < size; i++)
    d->keys[i] = list_new(DICT_KEY_COLLISIONS);

  d->pool = pool_new(sizeof(dict_key_value) * size, sizeof(dict_key_value));
  d->size = size;
  d->key_comparator = &default_key_comparator;
  d->hash_func = &default_hash_func;
  dict_init(d);
  return d;
}

SMALL_EXPORT void dict_set_key_comparator(dict *d, int (*comparator)(void *, void *))
{
  d->key_comparator = comparator;
}

SMALL_EXPORT void dict_set_hash_func(dict *d, int (*hash_func)(void *, int))
{
  d->hash_func = hash_func;
}

static int strings_cmp(void *a, void *b)
{
  return strcmp((const char *)a, (const char *)b);
}

static int strings_hash(void *key, int size)
{
  const char *s = (const char *)key;
  int sum = 0;
  unsigned int i;

  assert(s);

  for (i=0; i < strlen(s); i++)
    sum += (int)s[i];

  return sum % size;
}

SMALL_EXPORT void dict_use_string_keys(dict *d)
{
  dict_set_key_comparator(d, &strings_cmp);
  dict_set_hash_func(d, &strings_hash);
}

SMALL_EXPORT void dict_destroy(dict *d)
{
  int i;

  assert(d);
  assert(d->keys);

  for (i=0; i < d->size; i++)
    list_destroy(d->keys[i]);

  free(d->keys);
  free(d);
}

static list *get_keys(dict *d, void *key)
{
  int pos = d->hash_func(key, d->size);
  return d->keys[pos];
}

static dict_key_value *key_value_for(dict *d, void *key, list **keys)
{
  dict_key_value *kv;
  list_item *item;

  *keys = get_keys(d, key);
  assert(*keys);
  list_for_each(item, kv, *keys) {
    if (kv && d->key_comparator(kv->key, key) == 0)
      return kv;
  }

  return NULL;
}

static void add_key_value(list *keys, dict_key_value *kv)
{
  if (list_full(keys))
    list_resize(keys, list_count(keys) * 2);
  list_append(keys, kv);
}

static void * remove_key_value(list *keys, dict_key_value *kv)
{
  return list_remove_by_value(keys, kv);
}

/* This returns:
 *  - the old value, if the key existed
 *  - the new value, if there was no key
 *  - NULL, if the dictionary is full
 */
SMALL_EXPORT void *dict_set(dict *d, void *key, void *value)
{
  void *old = NULL;
  list *keys = NULL;
  dict_key_value *kv = NULL;

  LOCK(d);

  if (d->count == d->size)
    goto out;

  kv = key_value_for(d, key, &keys);

  if (kv) {
    old = kv->value;
    kv->value = value;
  } else {
    kv = pool_get(d->pool);
    kv->key = key;
    old = kv->value = value;
    add_key_value(keys, kv);
    d->count++;
  }

out:
  UNLOCK(d);
  return old;
}


/* the value associated to the key, or NULL */
SMALL_EXPORT void * dict_get(dict *d, void *key)
{
  list *keys = NULL;
  dict_key_value *kv = NULL;
  void * value = NULL;

  LOCK(d);

  kv = key_value_for(d, key, &keys);
  if (kv)
    value = kv->value;

  UNLOCK(d);

  return value;
}

SMALL_EXPORT void * dict_unset(dict *d, void *key)
{
  list *keys = NULL;
  dict_key_value *kv = NULL;
  void * value = NULL;

  LOCK(d);

  kv = key_value_for(d, key, &keys);
  if (kv) {
    value = kv->value;
    remove_key_value(keys, kv);
    d->count--;
  }

  UNLOCK(d);

  return value;
}

static void * key_transform(list_item *item)
{
  dict_key_value *kv = (dict_key_value *)list_item_value(item);
  return kv->key;
}

SMALL_EXPORT list *dict_keys(dict *d)
{
  list *keys;
  list *collision_list;
  int i;

  LOCK(d);
  keys = list_new(d->count);
  keys_for_each_list(d, i, collision_list) {
    list_concat_with_transform(keys, collision_list, key_transform);
  }
  UNLOCK(d);

  return keys;
}

list *dict_values(dict *d)
{
  return NULL;
}

SMALL_EXPORT int dict_count(dict *d)
{
  return d->count;
}
