#ifndef _SLAB_H_
#define _SLAB_H_

typedef struct slab slab;

void slab_init(slab *s);
slab *slab_new(int size);
void slab_destroy(slab *s);
void * slab_get_mem(slab *s);
void * slab_get_cur(slab *s);
int slab_get_size(slab *s);
void slab_update_position(slab *s, int bytes);
int slab_get_position(slab *s);
int slab_eof(slab *s);

#endif
