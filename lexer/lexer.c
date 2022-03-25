#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include "bufferReader/bufferReader.h"

struct lexer {
    BufferReader* bufferReader;    
};

Lexer* lexer_init(const char* sourceFilePath, size_t bufferSize) {
    Lexer* l = (Lexer*) malloc(sizeof(Lexer));

    if (l != NULL) {
        l->bufferReader = bufferReader_init(sourceFilePath, bufferSize);
    }

    return l;
}

Token lexer_getNextToken(Lexer *l) {
    Token t;
    t.type = S_OPEN_PARENTHESIS;

    bufferReader_test(l->bufferReader);

    return t;
}

void lexer_free(Lexer* l) {
    bufferReader_free(l->bufferReader);
    free(l);
}
