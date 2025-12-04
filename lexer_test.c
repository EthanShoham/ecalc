#include "lexer.h"
#include "token_list.h"
#include "char_reader.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  TokenType type;
  const char *lexeme;
} ExpectedToken;

static void run_lex_test(const char *input, const ExpectedToken *expected, size_t expected_count) {
  CharReader reader = {0};
  char_reader_init(&reader);
  assert(char_reader_add(&reader, input));

  TokenList tokens = lex_char_reader(&reader);
  size_t count = token_list_get_count(&tokens);
  assert(count == expected_count);

  for (size_t i = 0; i < expected_count; i++) {
    Token token = token_list_get_token_at(&tokens, i);
    assert(token.type == expected[i].type);
    if (expected[i].lexeme == NULL) {
      assert(token.lexeme == NULL);
    } else {
      assert(token.lexeme != NULL);
      assert(strcmp(token.lexeme, expected[i].lexeme) == 0);
    }
  }

  token_list_distroy(&tokens);
  char_reader_destroy(&reader);
}

static void test_simple_expression(void) {
  ExpectedToken expected[] = {
      {NUMBER_TOKEN, "1"}, {PLUS_TOKEN, "+"}, {NUMBER_TOKEN, "2"},
      {EOI_TOKEN, NULL}};
  run_lex_test("1 + 2", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_decimal_zero(void) {
  ExpectedToken expected[] = {{NUMBER_TOKEN, "."}, {EOI_TOKEN, NULL}};
  run_lex_test(" . ", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_identifier_and_ops(void) {
  ExpectedToken expected[] = {{IDENTIFIER_TOKEN, "foo123"},
                              {MULTIPLY_TOKEN, "*"},
                              {NUMBER_TOKEN, "."},
                              {PLUS_TOKEN, "+"},
                              {IDENTIFIER_TOKEN, "bar"},
                              {EOI_TOKEN, NULL}};
  run_lex_test("foo123 * . + bar", expected,
               sizeof(expected) / sizeof(expected[0]));
}

static void test_exponent_sequence(void) {
  ExpectedToken expected[] = {{NUMBER_TOKEN, "1"},
                              {EXPONENT_TOKEN, "e"},
                              {NUMBER_TOKEN, "10"},
                              {EOI_TOKEN, NULL}};
  run_lex_test("1e10", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_invalid_token(void) {
  ExpectedToken expected[] = {{NUMBER_TOKEN, "1"},
                              {INVALID_TOKEN, "$"},
                              {NUMBER_TOKEN, "2"},
                              {EOI_TOKEN, NULL}};
  run_lex_test("1$2", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_grouping_tokens(void) {
  ExpectedToken expected[] = {
      {LPAREN_TOKEN, "("},     {LBRACE_TOKEN, "{"}, {LBRACKET_TOKEN, "["},
      {IDENTIFIER_TOKEN, "x"}, {RBRACKET_TOKEN, "]"}, {RBRACE_TOKEN, "}"}, {RPAREN_TOKEN, ")"},
      {EOI_TOKEN, NULL}};
  run_lex_test("({[x]})", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_empty_input(void) {
  ExpectedToken expected[] = {{EOI_TOKEN, NULL}};
  run_lex_test("", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_power_operator(void) {
  ExpectedToken expected[] = {{POWER_TOKEN, "**"}, {EOI_TOKEN, NULL}};
  run_lex_test("**", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_number_then_identifier(void) {
  ExpectedToken expected[] = {{NUMBER_TOKEN, "12"},
                              {IDENTIFIER_TOKEN, "abc"},
                              {EOI_TOKEN, NULL}};
  run_lex_test("12abc", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_exponent_with_sign(void) {
  ExpectedToken expected[] = {{NUMBER_TOKEN, "3"},
                              {EXPONENT_TOKEN, "e"},
                              {MINUS_TOKEN, "-"},
                              {NUMBER_TOKEN, "2"},
                              {EOI_TOKEN, NULL}};
  run_lex_test("3e-2", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_only_whitespace(void) {
  ExpectedToken expected[] = {{EOI_TOKEN, NULL}};
  run_lex_test("   \t\n", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_consecutive_dots(void) {
  ExpectedToken expected[] = {{NUMBER_TOKEN, "."},
                              {NUMBER_TOKEN, "."},
                              {EOI_TOKEN, NULL}};
  run_lex_test("..", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_exponent_with_plus_no_digits(void) {
  ExpectedToken expected[] = {{NUMBER_TOKEN, "1"},
                              {EXPONENT_TOKEN, "e"},
                              {PLUS_TOKEN, "+"},
                              {EOI_TOKEN, NULL}};
  run_lex_test("1e+", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_trailing_e_identifier(void) {
  ExpectedToken expected[] = {{NUMBER_TOKEN, "1"},
                              {IDENTIFIER_TOKEN, "e"},
                              {EOI_TOKEN, NULL}};
  run_lex_test("1e", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_number_e_then_identifier(void) {
  ExpectedToken expected[] = {{NUMBER_TOKEN, "1"},
                              {IDENTIFIER_TOKEN, "efoo"},
                              {EOI_TOKEN, NULL}};
  run_lex_test("1efoo", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_number_eE_digits_identifier(void) {
  ExpectedToken expected[] = {{NUMBER_TOKEN, "1"},
                              {IDENTIFIER_TOKEN, "eE2"},
                              {EOI_TOKEN, NULL}};
  run_lex_test("1eE2", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_identifier_e_with_digits(void) {
  ExpectedToken expected[] = {{IDENTIFIER_TOKEN, "e2"}, {EOI_TOKEN, NULL}};
  run_lex_test("e2", expected, sizeof(expected) / sizeof(expected[0]));
}

static void test_identifier_e_alone(void) {
  ExpectedToken expected[] = {{IDENTIFIER_TOKEN, "e"}, {EOI_TOKEN, NULL}};
  run_lex_test("e", expected, sizeof(expected) / sizeof(expected[0]));
}

int main(void) {
  test_simple_expression();
  test_decimal_zero();
  test_identifier_and_ops();
  test_exponent_sequence();
  test_invalid_token();
  test_grouping_tokens();
  test_empty_input();
  test_power_operator();
  test_number_then_identifier();
  test_exponent_with_sign();
  test_exponent_with_plus_no_digits();
  test_trailing_e_identifier();
  test_number_e_then_identifier();
  test_number_eE_digits_identifier();
  test_identifier_e_with_digits();
  test_identifier_e_alone();
  test_only_whitespace();
  test_consecutive_dots();

  printf("All lexer tests passed\n");
  return 0;
}
