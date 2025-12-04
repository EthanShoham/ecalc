#ifndef TOKEN_LIST
#define TOKEN_LIST
#include <stddef.h>

typedef enum TokenType {
  NUMBER_TOKEN,     // 123 123.313 .3123 0.1231 11,222,333 11,222.333444
  IDENTIFIER_TOKEN, // x y z
  PLUS_TOKEN,       // +
  MINUS_TOKEN,      // -
  MULTIPLY_TOKEN,   // *
  DIVIDE_TOKEN,     // /
  MODULO_TOKEN,     // %
  POWER_TOKEN,      // ^ **
  EXPONENT_TOKEN,   // 'eE' right after a number, expect another number after
  LPAREN_TOKEN,     // (
  RPAREN_TOKEN,     // )
  LBRACKET_TOKEN,   // [
  RBRACKET_TOKEN,   // ]
  LBRACE_TOKEN,     // {
  RBRACE_TOKEN,     // }
  INVALID_TOKEN,
  EOI_TOKEN
} TokenType;

typedef struct Token {
  const TokenType type;
  const char *lexeme;
} Token;

typedef struct TokenList {
  const Token *_inner_token_list;
  const char *_inner_lexemes_container;
} TokenList;

void token_list_distroy(TokenList *token_list);
Token token_list_get_token_at(TokenList *token_list, size_t index);
size_t token_list_get_count(TokenList *token_list);

#endif
