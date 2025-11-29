#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef NDEBUG
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define assert_not_null_parameter(pointer) do{ const char* template = "NULL pointer parameter was passed to: %s on line: %d"; char message[200]; sprintf(message, template, __func__, __LINE__); assert(pointer && message); }while(0)
#else
#define assert_not_null_parameter(pointer) do{ const char* template = "NULL pointer parameter was passed to: %s on line: %d"; int count = strlen(template) + strlen(__func__) + 25 + 1; char message[count]; sprintf(message, template, __func__, __LINE__); assert(pointer && message); }while(0)
#endif
#else
#define assert_not_null_parameter(pointer) ((void)0)
#endif

// ----------------------------------- LIST START ----------------------------
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

void *list_alloc(size_t item_size_bytes, size_t init_capacity) {
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

void list_free(void* list) {
    assert_not_null_parameter(list);
    list_header* header = ((list_header*)list) - 1;
    free(header);
}

void *list_resize(void *list, size_t new_capacity) {
  assert_not_null_parameter(list);
  list_header *header = ((list_header *)list) - 1;
  if (header->capacity == new_capacity) {
    return header;
  }

  header->capacity = new_capacity;
  if (header->count > new_capacity) {
    header->count = new_capacity;
  }

  size_t new_mem_block_size =
      sizeof(list_header) + (header->item_size * new_capacity);
  list_header* new_header = realloc(header, new_mem_block_size);
  if (new_header == NULL) {
      return NULL;
  }

  return new_header + 1;
}

size_t list_count(void *list) {
  assert_not_null_parameter(list);
  list_header *header = ((list_header *)list) - 1;
  return header->count;
}

size_t list_capacity(void *list) {
  assert_not_null_parameter(list);
  list_header *header = ((list_header *)list) - 1;
  return header->capacity;
}

void *list_add(void *list, void *item_ref) {
  assert_not_null_parameter(list);
  list_header *header = ((list_header *)list) - 1;

  if (header->count < header->capacity) {
    void *dest = (char*)list + (header->count * header->item_size);
    if (memcpy_s(dest, header->item_size, item_ref, header->item_size) != 0) {
      return NULL;
    }

    header->count++;
    return list;
  }

  void *new_list = list_resize(list, header->capacity * 2);
  if (new_list == NULL) {
    return NULL;
  }

  list_header *new_header = ((list_header *)new_list) - 1;
  void *dest = (char*)new_list + (new_header->count * new_header->item_size);

  if(memcpy_s(dest, new_header->item_size, item_ref, new_header->item_size) !=
      0) {
        printf("Failed memcpy to the new buffer in list_add");
        exit(1);
  }

  new_header->count++;
  return new_list;
}

// ----------------------------------- LIST END ----------------------------------

// ----------------------------------- CHAR READER STRAT -------------------------

typedef struct CharReader {
  size_t current_buffer_index;
  size_t current_read_position;
  char const **buffers_list;
} CharReader;

void char_reader_init(CharReader *reader) {
  assert_not_null_parameter(reader);
  reader->current_read_position = 0;
  reader->current_buffer_index = 0;

  reader->buffers_list = list_alloc(sizeof(char *), 10);
}

void char_reader_destroy(CharReader *reader) {
  assert_not_null_parameter(reader);
  list_free(reader->buffers_list);
  reader->buffers_list = 0;
  reader->current_read_position = 0;
  reader->current_buffer_index = 0;
}

char char_reader_read(CharReader *reader) {
  assert_not_null_parameter(reader);
  char current_char = reader->buffers_list[reader->current_buffer_index]
                                          [reader->current_read_position];
  while (current_char == '\0') {
    if (reader->current_buffer_index == list_count(reader->buffers_list) - 1) {
        return '\0';
    }

    reader->current_buffer_index++;
    reader->current_read_position = 0;
    current_char = reader->buffers_list[reader->current_buffer_index][0];
  }

  reader->current_read_position++;
  return current_char;
}

bool char_reader_add(CharReader *reader, const char *str) {
  assert_not_null_parameter(reader);
  assert_not_null_parameter(reader->buffers_list &&
                            "reader was not initialized!");
  assert_not_null_parameter(str &&
                            " don't pass null str to add to the CharReader");

  char const **new_list = list_add(reader->buffers_list, &str);
  if (new_list == NULL) {
    return false;
  }

  reader->buffers_list = new_list;
  return true;
}

// ----------------------------------- CHAR READER END ---------------------------

int main(int argc, const char *argv[]) {
  CharReader reader = {0};
  char_reader_init(&reader);

  for (size_t i = 0; i < argc; i++) {
    char_reader_add(&reader, argv[i]);
    if(i < argc - 1) {
      char_reader_add(&reader, " ");
    }
  }

  for (char current = char_reader_read(&reader); current != 0;
       current = char_reader_read(&reader)) {
    putchar(current);
  }

  for (size_t i = 0; i < 5; i++)
  {
      char current = char_reader_read(&reader);
  }

  char_reader_destroy(&reader);
}
