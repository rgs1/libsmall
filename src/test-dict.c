#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <small/dict.h>
#include <small/list.h>
#include <small/util.h>


static void test_basic(void)
{
  dict *d = dict_new(10);
  list_t keys;

  dict_set(d, "hello", "goodbye");

  info("dict has %d keys", dict_count(d));
  assert(dict_count(d) == 1);
  assert(strcmp(dict_get(d, "hello"), "goodbye") == 0);
  assert(dict_get(d, "nokey") == NULL);

  dict_set(d, "hello", "updated");

  info("dict has %d keys", dict_count(d));
  assert(dict_count(d) == 1);
  assert(strcmp(dict_get(d, "hello"), "updated") == 0);

  dict_unset(d, "hello");

  info("dict has %d keys", dict_count(d));
  assert(dict_count(d) == 0);
  assert(dict_get(d, "hello") == NULL);

  dict_set(d, "a", "1");
  dict_set(d, "b", "2");
  keys = dict_keys(d);

  info("dict has %d keys", dict_count(d));

  assert(list_contains(keys, "a") > 0);
  assert(list_contains(keys, "b") > 0);

  list_destroy(keys);
}

static void test_string_keys(void)
{
  dict *d = dict_new(10);

  dict_use_string_keys(d);

  dict_set(d, "hello", "goodbye");
  info("dict has %d keys", dict_count(d));
  assert(dict_count(d) == 1);
  assert(strcmp(dict_get(d, strdup("hello")), "goodbye") == 0);
  dict_set(d, strdup("hello"), "updated");
  assert(dict_count(d) == 1);
  assert(strcmp(dict_get(d, strdup("hello")), "updated") == 0);
}

static char * randstr(void)
{
  static char buf[40];
  sprintf(buf, "abcdefghijklmnopqrstuvwyz0123456789");
  return strdup(strfry(buf));
}

static void test_big_dict(void)
{
  char *k;
  int num_keys = 1 << 13;
  dict *d = dict_new(num_keys);
  int i;
  list_t keys = list_new(num_keys);
  list_item_t item;

  dict_use_string_keys(d);

  /* add keys */
  for (i=0; i < num_keys; i++) {
    k = randstr();
    dict_set(d, k, k);
    list_append(keys, k);
  }

  info("dict has %d keys", dict_count(d));
  assert(dict_count(d) > 1);

  /* check keys are there */
  list_for_each(item, k, keys) {
    assert(strcmp(dict_get(d, k), k) == 0);
  }
}

int main(int argc, char **argv)
{
  run_test("basic: add, set, get & remove", &test_basic);
  run_test("string keys", &test_string_keys);
  run_test("big dict", &test_big_dict);

  return 0;
}
