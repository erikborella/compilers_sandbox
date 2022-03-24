#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

#define CODE_SOURCE_FILE "code_example.txt"

int main() {

    Lexer* l = lexer_init(CODE_SOURCE_FILE, 64);

    Token t = lexer_getNextToken(l);

    lexer_free(l);

    return 0;
}