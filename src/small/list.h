#ifndef _LIST_H_
#define _LIST_H_

typedef struct list list;
typedef struct list_item list_item;


list *list_new(int size);
void list_resize(list *l, int new_size);
void list_destroy(list *l);
list_item *list_head(list *l);
void *list_item_value(list_item *item);
list_item *list_item_next(list_item *item);
void list_init(list *l);
void * list_prepend(list *l, void *value);
void * list_append(list *l, void *value);
void list_concat(list *left, list *right);
void list_concat_with_transform(list *left,
                                list *right,
                                void *(*transform)(list_item *));
void * list_get(list *l, int pos);
void * list_remove_by_value(list *l, void *value);
void * list_remove_by_pos(list *l, int pos);
int list_contains(list *l, void *value);
int list_count(list *l);
int list_full(list *l);
void list_set_user_data(list *l, void *data);
void * list_get_user_data(list *l);

#define list_for_each_item(item, l)                                 \
    for (item = list_head(l); item != NULL; item = list_item_next(item))

#define list_for_each(item, v, l) \
  for (item = list_head(l), v = list_item_value(item); \
       item != NULL; \
       item = list_item_next(item), v = list_item_value(item))

#endif
