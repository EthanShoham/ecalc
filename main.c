#include <assert.h>
#include <stdio.h>
#include "char_reader.h"

enum TokenType {
  NUMBER, // 123 123.313 .3123 0.1231 11,222,333 11,222.333444

  IDENTIFIER, // x y z

  PLUS, // +
  MINUS,// -
  MULTIPLY, // *
  DIVIDE,// /
  MODULO,// %
  POWER,// ^ **

  LPAREN,// (
  RPAREN,// )

  LBRACKET, // [
  RBRACKET, // ]

  LBRACE, // {
  RBRACE, // }

  ERROR,
  EOI
};

typedef struct Lexer{
  int a;
} Lexer;

typedef enum State {
  START
} State;

State _lexer_start(Lexer* this, char c){
  return START;
}

static State (*state_functions[])(Lexer* this, char c) = {
  [START] = _lexer_start 
};

int main(int argc, const char *argv[]) {
  CharReader reader = {0};
  char_reader_init(&reader);

  for (size_t i = 0; i < argc; i++) {
    assert(char_reader_add(&reader, argv[i]));
    if (i < argc - 1) {
      assert(char_reader_add(&reader, " "));
    }
  }

  Lexer lexer = {0};
  State state = START;
  for(char c = char_reader_read(&reader); c != '\0';c = char_reader_read(&reader)) {
    state = state_functions[state](&lexer, c);
  }

  char_reader_destroy(&reader);
}
