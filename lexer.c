#include "lexer.h"
#include "list.h"
#include "token_list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Lexer {
  list_(Token) token_list;
  list_(char) lexemes_container;
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
  LPAREN,   // (
  RPAREN,   // )
  LBRACKET, // [
  RBRACKET, // ]
  LBRACE,   // {
  RBRACE,   // }
} State;

static bool _lexer_init(Lexer *this) {
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

static TokenList _lexer_destroy(Lexer *this) {
  assert(this && "_lexer_destroy(): this arg was null");
  assert(this->token_list && this->lexemes_container && "_lexer_destroy(): destroied a not init lexer");

  TokenList token_list = {._inner_token_list = this->token_list,
                          ._inner_lexemes_container = this->lexemes_container};

  this->token_list = NULL;
  this->lexemes_container = NULL;
  return token_list;
}

static void _lexer_add_char(Lexer *this, char c) {
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

  for (size_t i = 0; i < list_get_count(this->token_list); i++) {
    size_t new_index = this->token_list[i].lexeme - this->lexemes_container;
    this->token_list[i].lexeme = &new_container[new_index];
  }

  this->lexemes_container = new_container;
}

static void _lexer_add_token(Lexer *this, Token token) {
  assert(this && "_lexer_add_token(): arg this was null");
  this->token_list = list_add(this->token_list, &token);
  if (this->token_list == NULL) {
    fprintf(stderr, "_lexer_add_token(): failed to add token to token_list");
    exit(1);
  }
}

static void _lexer_cut_token(Lexer *this, TokenType cut_type) {
  _lexer_add_char(this, '\0');
  Token token = {
      .type = cut_type,
      .lexeme = &this->lexemes_container[this->current_lexeme_start_index]};
  _lexer_add_token(this, token);
  this->current_lexeme_start_index = list_get_count(this->lexemes_container);
}

static State _lexer_start(Lexer *this, char c) {
  if (c == ' ') {
    return START;
  }

  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_number(Lexer *this, char c) {
  if (c == ' ') {
    _lexer_cut_token(this, NUMBER_TOKEN);
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

  _lexer_cut_token(this, NUMBER_TOKEN);
  _lexer_add_char(this, c);
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return (c == 'e' || c == 'E') ? EXPONENT : IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_decimal(Lexer *this, char c) {
  if (c == ' ') {
    _lexer_cut_token(this, NUMBER_TOKEN);
    return START;
  }
  if (c >= '0' && c <= '9') {
    _lexer_add_char(this, c);
    return DECIMAL;
  }

  _lexer_cut_token(this, NUMBER_TOKEN);
  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return (c == 'e' || c == 'E') ? EXPONENT : IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_identifier(Lexer *this, char c) {
  if (c == ' ') {
    _lexer_cut_token(this, IDENTIFIER_TOKEN);
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

  _lexer_cut_token(this, IDENTIFIER_TOKEN);
  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_plus(Lexer *this, char c) {
  _lexer_cut_token(this, PLUS_TOKEN);

  if (c == ' ') {
    return START;
  }

  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_minus(Lexer *this, char c) {
  _lexer_cut_token(this, MINUS_TOKEN);

  if (c == ' ') {
    return START;
  }

  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_multiply(Lexer *this, char c) {
  if (c == ' ') {
    _lexer_cut_token(this, MULTIPLY_TOKEN);
    return START;
  }
  if (c == '*') {
    _lexer_add_char(this, c);
    return POWER;
  }

  _lexer_cut_token(this, MULTIPLY_TOKEN);
  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_exponent(Lexer *this, char c) {
  if (c == ' ') {
    _lexer_cut_token(this, IDENTIFIER_TOKEN);
    return START;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    _lexer_add_char(this, c);
    return IDENTIFIER;
  }
  if (c == '/') {
    _lexer_cut_token(this, IDENTIFIER_TOKEN);
    _lexer_add_char(this, c);
    return DIVIDE;
  }
  if (c == '%') {
    _lexer_cut_token(this, IDENTIFIER_TOKEN);
    _lexer_add_char(this, c);
    return MODULO;
  }
  if (c == '*') {
    _lexer_cut_token(this, IDENTIFIER_TOKEN);
    _lexer_add_char(this, c);
    return MULTIPLY;
  }
  if (c == '^') {
    _lexer_cut_token(this, IDENTIFIER_TOKEN);
    _lexer_add_char(this, c);
    return POWER;
  }
  if (c == ')') {
    _lexer_cut_token(this, IDENTIFIER_TOKEN);
    _lexer_add_char(this, c);
    return RPAREN;
  }
  if (c == ']') {
    _lexer_cut_token(this, IDENTIFIER_TOKEN);
    _lexer_add_char(this, c);
    return RBRACKET;
  }
  if (c == '}') {
    _lexer_cut_token(this, IDENTIFIER_TOKEN);
    _lexer_add_char(this, c);
    return RBRACE;
  }

  _lexer_cut_token(this, EXPONENT_TOKEN);
  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_divide(Lexer *this, char c) {
  _lexer_cut_token(this, DIVIDE_TOKEN);

  if (c == ' ') {
    return START;
  }

  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_modulo(Lexer *this, char c) {
  _lexer_cut_token(this, MODULO_TOKEN);

  if (c == ' ') {
    return START;
  }

  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_power(Lexer *this, char c) {
  _lexer_cut_token(this, POWER_TOKEN);

  if (c == ' ') {
    return START;
  }

  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_lparen(Lexer *this, char c) {
  _lexer_cut_token(this, LPAREN_TOKEN);

  if (c == ' ') {
    return START;
  }

  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_rparen(Lexer *this, char c) {
  _lexer_cut_token(this, RPAREN_TOKEN);

  if (c == ' ') {
    return START;
  }

  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_lbracket(Lexer *this, char c) {
  _lexer_cut_token(this, LBRACKET_TOKEN);

  if (c == ' ') {
    return START;
  }

  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_rbracket(Lexer *this, char c) {
  _lexer_cut_token(this, RBRACKET_TOKEN);

  if (c == ' ') {
    return START;
  }

  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_lbrace(Lexer *this, char c) {
  _lexer_cut_token(this, LBRACE_TOKEN);

  if (c == ' ') {
    return START;
  }

  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State _lexer_rbrace(Lexer *this, char c) {
  _lexer_cut_token(this, RBRACE_TOKEN);

  if (c == ' ') {
    return START;
  }

  _lexer_add_char(this, c);
  if (c == '.') {
    return DECIMAL;
  }
  if (c >= '0' && c <= '9') {
    return NUMBER;
  }
  bool is_identifier =
      (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_');
  if (is_identifier) {
    return IDENTIFIER;
  }
  if (c == '+') {
    return PLUS;
  }
  if (c == '-') {
    return MINUS;
  }
  if (c == '*') {
    return MULTIPLY;
  }
  if (c == '/') {
    return DIVIDE;
  }
  if (c == '%') {
    return MODULO;
  }
  if (c == '^') {
    return POWER;
  }
  if (c == '(') {
    return LPAREN;
  }
  if (c == ')') {
    return RPAREN;
  }
  if (c == '[') {
    return LBRACKET;
  }
  if (c == ']') {
    return RBRACKET;
  }
  if (c == '{') {
    return LBRACE;
  }
  if (c == '}') {
    return RBRACE;
  }
  _lexer_cut_token(this, INVALID_TOKEN);
  return START;
}

static State (*_lexer_state_functions[])(Lexer *this, char c) = {
    [START] = _lexer_start,       [NUMBER] = _lexer_number,
    [DECIMAL] = _lexer_decimal,   [IDENTIFIER] = _lexer_identifier,
    [PLUS] = _lexer_plus,         [MINUS] = _lexer_minus,
    [MULTIPLY] = _lexer_multiply, [DIVIDE] = _lexer_divide,
    [MODULO] = _lexer_modulo,     [POWER] = _lexer_power,
    [EXPONENT] = _lexer_exponent, [LPAREN] = _lexer_lparen,
    [RPAREN] = _lexer_rparen,     [LBRACKET] = _lexer_lbracket,
    [RBRACKET] = _lexer_rbracket, [LBRACE] = _lexer_lbrace,
    [RBRACE] = _lexer_rbrace};

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
  _lexer_state_functions[state](&lexer, ' ');

  Token end_token = {.lexeme = NULL, .type = EOI_TOKEN};
  list_(Token) token_list = list_add(lexer.token_list, &end_token);
  if (token_list == NULL) {
    fprintf(stderr, "failed to add the EOF token");
    exit(1);
  }


  lexer.token_list = token_list;
  return _lexer_destroy(&lexer);
}
