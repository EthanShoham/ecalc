#!/usr/bin/env sh
set -e

CFLAGS="-std=c11 -Wall -Wextra -Werror"
SHARED_SOURCES="lexer.c char_reader.c token_list.c list.c"

echo "Building main.exe..."
gcc $CFLAGS main.c $SHARED_SOURCES -o main.exe

echo "Building lexer_test.exe..."
gcc $CFLAGS lexer_test.c $SHARED_SOURCES -o lexer_test.exe

echo "Running lexer_test.exe..."
./lexer_test.exe