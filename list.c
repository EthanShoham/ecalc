#include <assert.h>
#include <stdlib.h>
#include <string.h>

// on 64 bit systems
//  0                   1                   2                   3
//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   item_size   |     count     |    capacity   |   list start  | ....
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
typedef struct list_header {
  size_t item_size;
  size_t count;
  size_t capacity;
} list_header;

static size_t _list_max_capacity(size_t item_size) {
  assert(item_size && "_list_max_capacity(): item_size can not be zero");
  return (SIZE_MAX - sizeof(list_header)) / item_size;
}

static void *_list_resize(void *list, size_t new_capacity) {
  assert(list && "list_resize(): parameter list was null");
  list_header *header = ((list_header *)list) - 1;
  if (header->capacity == new_capacity) {
    return header + 1;
  }

  if (new_capacity > _list_max_capacity(header->item_size)) {
    return NULL;
  }

  size_t new_mem_block_size =
      sizeof(list_header) + (header->item_size * new_capacity);
  list_header *new_header = realloc(header, new_mem_block_size);
  if (new_header == NULL) {
    return NULL;
  }

  new_header->capacity = new_capacity;
  if (new_header->count > new_capacity) {
    new_header->count = new_capacity;
  }

  return new_header + 1;
}

void *list_alloc(size_t item_size_bytes, size_t init_capacity) {
  assert(item_size_bytes && "why would you create zero bytes item list??");
  if (item_size_bytes == 0) {
    return NULL;
  }

  if (init_capacity > _list_max_capacity(item_size_bytes)) {
    return NULL;
  }

  size_t mem_block_size =
      sizeof(list_header) + (item_size_bytes * init_capacity);
  list_header *header = malloc(mem_block_size);
  if (header == NULL) {
    return NULL;
  }

  header->item_size = item_size_bytes;
  header->capacity = init_capacity;
  header->count = 0;
  return header + 1;
}

void list_free(void *list) {
  assert(list && "list_free(): parameter list was null");
  list_header *header = ((list_header *)list) - 1;
  free(header);
}

size_t list_get_count(const void *list) {
  assert(list && "list_count(): parameter list was null");
  list_header *header = ((list_header *)list) - 1;
  return header->count;
}

size_t list_get_capacity(const void *list) {
  assert(list && "list_capacity(): parameter list was null");
  list_header *header = ((list_header *)list) - 1;
  return header->capacity;
}

void *list_add(void *list, const void *item_ref) {
  assert(list && "list_add(): parameter list was null");
  assert(item_ref && "list_add(): parameter item_ref was null");
  list_header *header = ((list_header *)list) - 1;

  if (header->count == _list_max_capacity(header->item_size)) {
    return NULL;
  }

  if (header->count < header->capacity) {
    void *dest = (char *)list + (header->count * header->item_size);
    memcpy(dest, item_ref, header->item_size);
    header->count++;
    return list;
  }

  size_t new_capacity = header->capacity == 0 ? 1 : header->capacity * 2;
  size_t new_size = (new_capacity * header->item_size) + sizeof(list_header);
  size_t old_size =
      (header->capacity * header->item_size) + sizeof(list_header);
  if (new_size < old_size) {
    new_capacity = _list_max_capacity(header->item_size);
  }

  void *new_list = _list_resize(list, new_capacity);
  if (new_list == NULL) {
    return NULL;
  }

  list_header *new_header = ((list_header *)new_list) - 1;
  void *dest = (char *)new_list + (new_header->count * new_header->item_size);
  memcpy(dest, item_ref, new_header->item_size);
  new_header->count++;
  return new_list;
}
