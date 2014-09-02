#ifndef _LATCH_H_
#define _LATCH_H_

typedef struct latch latch;

latch *latch_new(int count);
void latch_init(latch *l);
void latch_destroy(latch *l);
void latch_up(latch *l);
void latch_add(latch *l, int add);
void latch_down(latch *l);
void latch_wait(latch *l);
int latch_count(latch *l);
#endif
