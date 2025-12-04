#include "char_reader.h"
#include "lexer.h"
#include "token_list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static char *token_type_names[] = {
    [NUMBER_TOKEN] =
        "NUMBER_TOKEN", // 123 123.313 .3123 0.1231 11,222,333 11,222.333444
    [IDENTIFIER_TOKEN] = "IDENTIFIER_TOKEN", // x y z
    [PLUS_TOKEN] = "PLUS_TOKEN",             // +
    [MINUS_TOKEN] = "MINUS_TOKEN",           // -
    [MULTIPLY_TOKEN] = "MULTIPLY_TOKEN",     // *
    [DIVIDE_TOKEN] = "DIVIDE_TOKEN",         // /
    [MODULO_TOKEN] = "MODULO_TOKEN",         // %
    [POWER_TOKEN] = "POWER_TOKEN",           // ^ **
    [EXPONENT_TOKEN] = "EXPONENT_TOKEN", // 'eE' right after a number, expect
                                         // another number after
    [LPAREN_TOKEN] = "LPAREN_TOKEN",     // (
    [RPAREN_TOKEN] = "RPAREN_TOKEN",     // )
    [LBRACKET_TOKEN] = "LBRACKET_TOKEN", // [
    [RBRACKET_TOKEN] = "RBRACKET_TOKEN", // ]
    [LBRACE_TOKEN] = "LBRACE_TOKEN",     // {
    [RBRACE_TOKEN] = "RBRACE_TOKEN",     // }
    [INVALID_TOKEN] = "INVALID_TOKEN",
    [EOI_TOKEN] = "EOI_TOKEN"};

int main(int argc, const char *argv[]) {
  CharReader reader = {0};
  char_reader_init(&reader);

  for (int i = 1; i < argc; i++) {
    assert(char_reader_add(&reader, argv[i]));
    if (i < argc - 1) {
      assert(char_reader_add(&reader, " "));
    }
  }

  TokenList tokens = lex_char_reader(&reader);
  for (size_t i = 0; i < token_list_get_count(&tokens); i++) {
    Token token = token_list_get_token_at(&tokens, i);
    printf("{ type:\"%s\", lexeme:\"%s\" }\n", token_type_names[token.type],
           token.lexeme ? token.lexeme : "[NULL]");
  }

  token_list_distroy(&tokens);
  char_reader_destroy(&reader);
}
