#ifndef LIST
#define LIST
#include <stddef.h>

#define list(type, capacity) ((type*)list_alloc(sizeof(type), (capacity)))
#define list_(type) type*

void *list_alloc(size_t item_size_bytes, size_t init_capacity);
void list_free(void *list);
size_t list_get_count(const void *list);
size_t list_get_capacity(const void *list);
void *list_add(void *list, const void *item_ref);

#endif
