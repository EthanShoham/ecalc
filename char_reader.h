#ifndef CHAR_READER
#define CHAR_READER
#include <stdbool.h>
#include <stddef.h>

typedef struct CharReaderNode {
  char *str_buffer;
  struct CharReaderNode* next;
} CharReaderNode;

typedef struct CharReader {
  size_t current_read_position;
  CharReaderNode *head;
  CharReaderNode *tail;
} CharReader;

void char_reader_init(CharReader *reader);
void char_reader_destroy(CharReader *reader);
bool char_reader_add(CharReader *reader, const char *str);
char char_reader_read(CharReader *reader);

#endif
