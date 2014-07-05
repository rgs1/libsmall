#ifndef _QUEUE_H_
#define _QUEUE_H_

typedef struct queue queue;

queue *queue_new(int size);
void queue_destroy(queue *q);
void queue_init(queue *q);
int queue_add(queue *q, void *item);
void * queue_remove(queue *q);
int queue_empty(queue *q);
int queue_count(queue *q);
void queue_set_user_data(queue *q, void *data);
void *queue_get_user_data(queue *q);

#endif
