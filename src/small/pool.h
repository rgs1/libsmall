#ifndef _POOL_H_
#define _POOL_H_

typedef struct pool pool;

void pool_init(pool *p);
pool *pool_new(int size, int item_size);
void pool_destroy(pool *p);
void * pool_get(pool *p);
void pool_put(pool *p, void *item);
void pool_resize(pool *s, int new_size);

#endif
