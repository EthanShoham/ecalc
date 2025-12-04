#include "token_list.h"
#include "list.h"
#include <assert.h>

void token_list_distroy(TokenList *token_list) {
  assert(token_list && "token_list_distroy(): arg token_list was null");
  list_free((list_(Token) *)token_list->_inner_token_list);
  list_free((list_(Token) *)token_list->_inner_lexemes_container);
}

Token token_list_get_token_at(TokenList *token_list, size_t index) {
  assert(token_list && "token_list_get_token_at(): arg token_list was null");
  assert(index < list_get_count(token_list->_inner_token_list) &&
         "token_list_get_token_at(): index out of bounds");
  return token_list->_inner_token_list[index];
}

size_t token_list_get_count(TokenList *token_list) {
  assert(token_list && "token_list_get_count(): arg token_list was null");
  return list_get_count(token_list->_inner_token_list);
}
