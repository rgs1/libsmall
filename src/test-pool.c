#include <assert.h>
#include <stdlib.h>

#include <small/pool.h>
#include <small/slab.h>
#include <small/util.h>


static void test_basic(void)
{
  int i;
  void *start = NULL;
  void *item;
  pool_t p = pool_new(100, 10);

  /* get all items */
  for (i=0; i < 10; i++) {
    item = pool_get(p);
    if (!i)
      start = item;
    assert(item);
  }

  assert(pool_get(p) == NULL);

  /* put all items */
  for (i=0; i < 10; i++) {
    void *t = (void *)((char *)start + (i * 10));
    pool_put(p, t);
  }

  /* get them back */
  for (i=0; i < 10; i++)
    assert(pool_get(p));

  pool_destroy(p);
}

static void test_resize(void)
{
  void *a, *b, *c;
  pool_t p = pool_new(20, 10);

  a = pool_get(p);
  assert(a);

  b = pool_get(p);
  assert(b);

  assert(pool_get(p) == NULL);

  pool_resize(p, 30);

  c = pool_get(p);
  assert(c);

  assert(pool_get(p) == NULL);

  /* put them back */
  pool_put(p, a);
  pool_put(p, b);
  pool_put(p, c);

  /* get them again */
  a = pool_get(p);
  assert(a);

  b = pool_get(p);
  assert(b);

  c = pool_get(p);
  assert(c);

  assert(pool_get(p) == NULL);

  /* put them back, again */
  pool_put(p, a);
  pool_put(p, b);
  pool_put(p, c);

  pool_destroy(p);
}

int main(int argc, char **argv)
{
  run_test("basic", &test_basic);
  run_test("resize", &test_resize);

  return 0;
}
