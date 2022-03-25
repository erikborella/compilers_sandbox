#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

#define CODE_SOURCE_FILE "code_example.txt"

int main() {

    Lexer* l = lexer_init(CODE_SOURCE_FILE, 8);

    lexer_getNextToken(l);
    lexer_getNextToken(l);
    lexer_getNextToken(l);
    lexer_getNextToken(l);

    lexer_free(l);

    return 0;
}