#ifndef _WORKER_H_
#define _WORKER_H_

typedef struct worker worker;

worker *worker_new(void *(*loop)(void *), void *data);
worker *worker_create(void *(*loop)(void *), void *data);
void worker_run(worker *w);
void *worker_get_data(worker *w);
void worker_destroy(worker *w);

#endif
