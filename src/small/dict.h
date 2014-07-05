#ifndef _DICT_H_
#define _DICT_H_

#include "list.h"

typedef struct dict dict;

dict *dict_new(int size);
void dict_destroy(dict *d);
void dict_init(dict *d);
void * dict_set(dict *d, void *key, void *value);
void * dict_get(dict *d, void *key);
list *dict_keys(dict *d);
list *dict_values(dict *d);
void * dict_unset(dict *d, void *key);
int dict_count(dict *d);
void dict_set_key_comparator(dict *d, int (*comparator)(void *, void *));
void dict_set_hash_func(dict *d, int (*hash_func)(void *, int));
void dict_use_string_keys(dict *d);
void dict_set_user_data(dict *d, void *data);
void * dict_get_user_data(dict *q);

#endif
