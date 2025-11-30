#include "char_reader.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void char_reader_init(CharReader *reader) {
  assert(reader && "char_reader_init(): parameter reader was null");
  reader->current_read_position = 0;
  reader->head = NULL;
  reader->tail = NULL;
}

void char_reader_destroy(CharReader *reader) {
  assert(reader && "char_reader_destroy(): parameter reader was null");
  reader->current_read_position = 0;

  CharReaderNode *current = reader->head;
  while (current != NULL) {
    CharReaderNode *next = current->next;
    free(current->str_buffer);
    free(current);
    current = next;
  }

  reader->head = NULL;
  reader->tail = NULL;
}

bool char_reader_add(CharReader *reader, const char *str) {
  assert(reader && "char_reader_add(): parameter reader was null");
  assert(str && "char_reader_add(): parameter str was null");

  size_t str_size = strlen(str) + 1;
  char* str_cpy = malloc(str_size * sizeof(char));
  if(str_cpy == NULL) {
    return false;
  }

  memcpy(str_cpy, str, str_size * sizeof(char));

  CharReaderNode *new_node = malloc(sizeof(CharReaderNode));
  if(new_node == NULL) {
    free(str_cpy);
    return false;
  }

  new_node->str_buffer = str_cpy;
  new_node->next = NULL;

  CharReaderNode *prev = reader->tail;
  if(prev == NULL) {
    reader->head = new_node;
    reader->tail = new_node;
    return true;
  }

  prev->next = new_node;
  reader->tail = new_node;
  return true;
}

char char_reader_read(CharReader *reader) {
  assert(reader && "char_reader_read(): parameter reader was null");
  if(reader->head == NULL) {
    return '\0';
  }

  char current_char = reader->head->str_buffer[reader->current_read_position];
  while(current_char == '\0') {
    CharReaderNode *next = reader->head->next;

    free(reader->head->str_buffer);
    free(reader->head);

    reader->head = next;

    if(next == NULL) {
      reader->tail = NULL;
      reader->current_read_position = 0;
      return '\0';
    }

    reader->current_read_position = 0;
    current_char = reader->head->str_buffer[reader->current_read_position];
  }

  reader->current_read_position++;
  return current_char;
}
