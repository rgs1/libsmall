/*-*- Mode: C; c-basic-offset: 2; indent-tabs-mode: nil -*-*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <small/list.h>
#include <small/pool.h>
#include <small/util.h>


static void test_add(void)
{
  list *l = list_new(10);
  list_prepend(l, "hello");
  list_append(l, "goodbye");
  info("list has %d items", list_count(l));
  assert(list_count(l) == 2);
}

static void test_add_no_space(void)
{
  list *l = list_new(1);

  assert(list_prepend(l, "hello"));
  assert(list_append(l, "goodbye") == NULL);
  info("list has %d items", list_count(l));
  assert(list_count(l) == 1);
}

static void test_remove(void)
{
  list *l = list_new(10);

  list_prepend(l, "hello");
  list_append(l, "goodbye");
  info("list has %d items", list_count(l));
  assert(list_count(l) == 2);

  list_remove_by_value(l, "goodbye");
  info("list has %d items, head = %s", list_count(l), list_item_value(list_head(l)));
  assert(list_count(l) == 1);

  list_remove_by_value(l, "hello");
  info("list has %d items", list_count(l));
  assert(list_count(l) == 0);

  list_append(l, "florence");
  list_append(l, "tuscany");
  assert(list_count(l) == 2);
  list_remove_by_pos(l, 0);
  info("list has %d items", list_count(l));
  assert(list_count(l) == 1);
  list_remove_by_pos(l, 0);
  info("list has %d items", list_count(l));
  assert(list_count(l) == 0);
}

static void test_get(void)
{
  list *l = list_new(10);

  list_append(l, "one");
  list_append(l, "two");
  list_append(l, "three");
  info("list has %d items", list_count(l));
  assert(list_count(l) == 3);

  assert(strcmp((const char *)list_get(l, 0), "one") == 0);
  assert(strcmp((const char *)list_get(l, 1), "two") == 0);
  assert(strcmp((const char *)list_get(l, 2), "three") == 0);
}

static void test_resize(void)
{
  list *l = list_new(2);

  list_append(l, "one");
  list_append(l, "two");
  info("list has %d items", list_count(l));
  assert(list_append(l, "three") == NULL);
  assert(list_count(l) == 2);

  list_resize(l, 4);
  list_append(l, "three");
  list_append(l, "four");
  info("list has %d items", list_count(l));
  assert(list_count(l) == 4);

  assert(strcmp((const char *)list_get(l, 0), "one") == 0);
  assert(strcmp((const char *)list_get(l, 1), "two") == 0);
  assert(strcmp((const char *)list_get(l, 2), "three") == 0);
  assert(strcmp((const char *)list_get(l, 3), "four") == 0);
}

int main(int argc, char **argv)
{
  run_test("add", &test_add);
  run_test("no more space", &test_add_no_space);
  run_test("remove", &test_remove);
  run_test("get by pos", &test_get);
  run_test("resize", &test_resize);

  return 0;
}
