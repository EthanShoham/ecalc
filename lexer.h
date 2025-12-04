#ifndef LEXER
#define LEXER
#include "char_reader.h"
#include "token_list.h"

TokenList lex_char_reader(CharReader *reader);
#endif
