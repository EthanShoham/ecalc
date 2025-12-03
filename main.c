#include "char_reader.h"
#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum TokenType {
  NUMBER_TOKEN, // 123 123.313 .3123 0.1231 11,222,333 11,222.333444

  IDENTIFIER_TOKEN, // x y z

  PLUS_TOKEN,     // +
  MINUS_TOKEN,    // -
  MULTIPLY_TOKEN, // *
  DIVIDE_TOKEN,   // /
  MODULO_TOKEN,   // %
  POWER_TOKEN,    // ^ **
  EXPONENT_TOKEN, // 'eE' right after a number, expect another number after

  LPAREN_TOKEN, // (
  RPAREN_TOKEN, // )

  LBRACKET_TOKEN, // [
  RBRACKET_TOKEN, // ]

  LBRACE_TOKEN, // {
  RBRACE_TOKEN, // }

  INVALID_TOKEN,
  EOI_TOKEN
} TokenType;

typedef struct Token {
  const char *lexeme;
  const TokenType type;
} Token;

typedef struct TokenList {
  const Token *_inner_token_list;
  const char *_inner_lexemes_container;
} TokenList;

typedef struct Lexer {
  Token *token_list;
  char *lexemes_container;
  size_t current_lexeme_start_index;
} Lexer;

typedef enum State {
  START,

  NUMBER,
  DECIMAL,

  IDENTIFIER,

  PLUS,     // +
  MINUS,    // -
  MULTIPLY, // *
  DIVIDE,   // /
  MODULO,   // %
  POWER,    // ^ **
  EXPONENT, // 'eE' right after a number, expect another number after

  LPAREN, // (
  RPAREN, // )

  LBRACKET, // [
  RBRACKET, // ]

  LBRACE, // {
  RBRACE, // }

  END
} State;

bool _lexer_init(Lexer *this) {
  assert(this && "_lexer_init(): this arg was null");
  assert(!this->token_list && !this->lexemes_container &&
         "_lexer_init(): init a not distroied lexer");

  this->token_list = list(Token, 25);
  if (this->token_list == NULL) {
    return false;
  }

  this->lexemes_container = list(char, 150);
  if (this->lexemes_container == NULL) {
    list_free(this->token_list);
    return false;
  }

  this->current_lexeme_start_index = 0;
  return true;
}

TokenList _lexer_destroy(Lexer *this) {
  assert(this && "_lexer_destroy(): this arg was null");
  assert(!this->token_list && !this->lexemes_container &&
         "_lexer_destroy(): destroied a not init lexer");

  TokenList token_list = {._inner_token_list = this->token_list,
                          ._inner_lexemes_container = this->lexemes_container};

  this->token_list = NULL;
  this->lexemes_container = NULL;
  return token_list;
}

void _lexer_add_char(Lexer *this, char c) {
  assert(this && "_lexer_add_token(): arg this was null");
  char *new_container = list_add(this->lexemes_container, &c);
  if (new_container == NULL) {
    fprintf(stderr,
            "_lexer_add_char(): failed to add char to lexemes_container");
    exit(1);
  }

  if (new_container == this->lexemes_container) {
    return;
  }

  for (size_t i = 0; i < list_count(this->token_list); i++) {
    size_t new_index = this->token_list[i].lexeme - this->lexemes_container;
    this->token_list[i].lexeme = &new_container[new_index];
  }

  this->lexemes_container = new_container;
}

void _lexer_add_token(Lexer *this, Token token) {
  assert(this && "_lexer_add_token(): arg this was null");
  this->token_list = list_add(this->token_list, &token);
  if (this->token_list == NULL) {
    fprintf(stderr, "_lexer_add_token(): failed to add token to token_list");
    exit(1);
  }
}

State _lexer_start(Lexer *this, char c) {
  if (c == ' ')
    return START;

  if (c == '.') {
    _lexer_add_char(this, c);
    return DECIMAL;
  }

  if (c >= '0' && c <= '9') {
    _lexer_add_char(this, c);
    return NUMBER;
  }

  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    _lexer_add_char(this, c);
    return IDENTIFIER;
  }

  if (c == '+') {
    _lexer_add_char(this, c);
    return PLUS;
  }

  if (c == '-') {
    _lexer_add_char(this, c);
    return MINUS;
  }

  if (c == '*') {
    _lexer_add_char(this, c);
    return MULTIPLY;
  }

  if (c == '/') {
    _lexer_add_char(this, c);
    return DIVIDE;
  }

  if (c == '%') {
    _lexer_add_char(this, c);
    return MODULO;
  }

  if (c == '^') {
    _lexer_add_char(this, c);
    return POWER;
  }

  if (c == '(') {
    _lexer_add_char(this, c);
    return LPAREN;
  }

  if (c == ')') {
    _lexer_add_char(this, c);
    return RPAREN;
  }

  if (c == '[') {
    _lexer_add_char(this, c);
    return LBRACKET;
  }

  if (c == ']') {
    _lexer_add_char(this, c);
    return RBRACKET;
  }

  if (c == '{') {
    _lexer_add_char(this, c);
    return LBRACE;
  }

  if (c == '}') {
    _lexer_add_char(this, c);
    return RBRACE;
  }

  size_t lexeme_index = list_count(this->lexemes_container);
  _lexer_add_char(this, c);
  _lexer_add_char(this, '\0');

  Token token = {.type = INVALID_TOKEN,
                 .lexeme = &this->lexemes_container[lexeme_index]};
  _lexer_add_token(this, token);
  this->current_lexeme_start_index = lexeme_index + 2;
  return START;
}

State _lexer_number(Lexer *this, char c) {
  if (c == ' ') {
    _lexer_add_char(this, '\0');
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container);
    return START;
  }

  if (c == '.') {
    _lexer_add_char(this, c);
    return DECIMAL;
  }

  if (c >= '0' && c <= '9') {
    _lexer_add_char(this, c);
    return NUMBER;
  }

  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;

    if (c == 'e' || c == 'E') {
      return EXPONENT;
    }

    return IDENTIFIER;
  }

  if (c == '+') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return PLUS;
  }

  if (c == '-') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return MINUS;
  }

  if (c == '*') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return MULTIPLY;
  }

  if (c == '/') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return DIVIDE;
  }

  if (c == '%') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return MODULO;
  }

  if (c == '^') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return POWER;
  }

  if (c == '(') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return LPAREN;
  }

  if (c == ')') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return RPAREN;
  }

  if (c == '[') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return LBRACKET;
  }

  if (c == ']') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return RBRACKET;
  }

  if (c == '{') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return LBRACE;
  }

  if (c == '}') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return RBRACE;
  }

  _lexer_add_char(this, '\0');
  size_t invalid_lexeme_index = list_count(this->lexemes_container);
  _lexer_add_char(this, c);
  _lexer_add_char(this, '\0');
  Token token = {
      .type = NUMBER_TOKEN,
      .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
  Token invalid_token = {.type = INVALID_TOKEN,
                         .lexeme =
                             &this->lexemes_container[invalid_lexeme_index]};
  _lexer_add_token(this, token);
  _lexer_add_token(this, token);
  this->current_lexeme_start_index = invalid_lexeme_index + 2;
  return START;
}

State _lexer_decimal(Lexer *this, char c) {
  if (c == ' ') {
    _lexer_add_char(this, '\0');
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container);
    return START;
  }

  if (c == '.') {
    _lexer_add_char(this, '\0');
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container);
    _lexer_add_char(this, c);
    _lexer_add_char(this, '\0');
    Token invalid_token = {
        .type = INVALID_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index += 2;
    return START;
  }

  if (c >= '0' && c <= '9') {
    _lexer_add_char(this, c);
    return DECIMAL;
  }

  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;

    if (c == 'e' || c == 'E') {
      return EXPONENT;
    }

    return IDENTIFIER;
  }

  if (c == '+') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return PLUS;
  }

  if (c == '-') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return MINUS;
  }

  if (c == '*') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return MULTIPLY;
  }

  if (c == '/') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return DIVIDE;
  }

  if (c == '%') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return MODULO;
  }

  if (c == '^') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return POWER;
  }

  if (c == '(') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return LPAREN;
  }

  if (c == ')') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return RPAREN;
  }

  if (c == '[') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return LBRACKET;
  }

  if (c == ']') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return RBRACKET;
  }

  if (c == '{') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return LBRACE;
  }

  if (c == '}') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = NUMBER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return RBRACE;
  }

  _lexer_add_char(this, '\0');
  size_t invalid_lexeme_index = list_count(this->lexemes_container);
  _lexer_add_char(this, c);
  _lexer_add_char(this, '\0');
  Token token = {
      .type = NUMBER_TOKEN,
      .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
  Token invalid_token = {.type = INVALID_TOKEN,
                         .lexeme =
                             &this->lexemes_container[invalid_lexeme_index]};
  _lexer_add_token(this, token);
  _lexer_add_token(this, token);
  this->current_lexeme_start_index = invalid_lexeme_index + 2;
  return START;
}

State _lexer_identifier(Lexer *this, char c) {
  if (c == ' ') {
    _lexer_add_char(this, '\0');
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container);
    return START;
  }

  if (c == '.') {
    _lexer_add_char(this, '\0');
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container);
    _lexer_add_char(this, c);
    _lexer_add_char(this, '\0');
    Token invalid_token = {
        .type = INVALID_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index += 2;
    return START;
  }

  if (c >= '0' && c <= '9') {
    _lexer_add_char(this, c);
    return IDENTIFIER;
  }

  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    _lexer_add_char(this, c);
    return IDENTIFIER;
  }

  if (c == '+') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return PLUS;
  }

  if (c == '-') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return MINUS;
  }

  if (c == '*') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return MULTIPLY;
  }

  if (c == '/') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return DIVIDE;
  }

  if (c == '%') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return MODULO;
  }

  if (c == '^') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return POWER;
  }

  if (c == '(') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return LPAREN;
  }

  if (c == ')') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return RPAREN;
  }

  if (c == '[') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return LBRACKET;
  }

  if (c == ']') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return RBRACKET;
  }

  if (c == '{') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return LBRACE;
  }

  if (c == '}') {
    _lexer_add_char(this, '\0');
    _lexer_add_char(this, c);
    Token token = {
        .type = IDENTIFIER_TOKEN,
        .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
    _lexer_add_token(this, token);
    this->current_lexeme_start_index = list_count(this->lexemes_container) - 1;
    return RBRACE;
  }

  _lexer_add_char(this, '\0');
  size_t invalid_lexeme_index = list_count(this->lexemes_container);
  _lexer_add_char(this, c);
  _lexer_add_char(this, '\0');
  Token token = {
      .type = IDENTIFIER_TOKEN,
      .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
  Token invalid_token = {.type = INVALID_TOKEN,
                         .lexeme =
                             &this->lexemes_container[invalid_lexeme_index]};
  _lexer_add_token(this, token);
  _lexer_add_token(this, token);
  this->current_lexeme_start_index = invalid_lexeme_index + 2;
  return START;
}

static State (*_lexer_state_functions[])(Lexer *this,
                                         char c) = {[START] = _lexer_start};

TokenList lex_char_reader(CharReader *reader) {
  assert(reader && "lex_char_reader(): arg reader was null");
  Lexer lexer = {0};
  if (_lexer_init(&lexer) == false) {
    fprintf(stderr, "failed to initialize lexer");
    exit(1);
  }

  State state = START;
  for (char c = char_reader_read(reader); c != '\0';
       c = char_reader_read(reader)) {
    state = _lexer_state_functions[state](&lexer, c);
  }

  Token end_token = {.lexeme = NULL, .type = EOI_TOKEN};
  Token *token_list = list_add(lexer.token_list, &end_token);
  if (token_list == NULL) {
    fprintf(stderr, "failed to add the EOF token");
    exit(1);
  }

  return _lexer_destroy(&lexer);
}

int main(int argc, const char *argv[]) {
  CharReader reader = {0};
  char_reader_init(&reader);

  for (size_t i = 0; i < argc; i++) {
    assert(char_reader_add(&reader, argv[i]));
    if (i < argc - 1) {
      assert(char_reader_add(&reader, " "));
    }
  }

  TokenList tokens = lex_char_reader(&reader);
  char_reader_destroy(&reader);
}
