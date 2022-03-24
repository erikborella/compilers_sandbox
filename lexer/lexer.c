#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

struct lexer {
    FILE* sourceFile;
    size_t bufferSize;
    char* buffer;
    bool loadFirstPart;
};

FILE* _openFileAsReadOrExitWithError(const char* sourceFilePath) {
    FILE* f = fopen(sourceFilePath, "r");

    if (f == NULL) {
        fprintf(stderr, "Lexer Error: Unable to open file \"%s\"\n", sourceFilePath);
        exit(1);
    }

    return f;
}

void* _mallocOrExitWithError(size_t size) {
    void* m = malloc(size);

    if (m == NULL) {
        fprintf(stderr, "Lexer Error: Unable to allocate %lu bytes\n", size);
    }

    return m;
}

void _loadChunk(Lexer* l) {
    size_t startPositionToLoad;
    size_t bytesRead;

    if (l->loadFirstPart)
        startPositionToLoad = 0;
    else
        startPositionToLoad = l->bufferSize;

    bytesRead = fread(l->buffer + startPositionToLoad, sizeof(char), l->bufferSize, l->sourceFile);

    if (bytesRead < l->bufferSize)
        *(l->buffer + startPositionToLoad + bytesRead) = 0;

    l->loadFirstPart = !l->loadFirstPart;
}

Lexer* lexer_init(const char* sourceFilePath, size_t bufferSize) {
    Lexer* l = (Lexer*) malloc(sizeof(Lexer));

    if (l != NULL) {
        l->sourceFile = _openFileAsReadOrExitWithError(sourceFilePath);
        l->bufferSize = bufferSize;
        //Double the buffer size to use the double buffer technique
        l->buffer = _mallocOrExitWithError(sizeof(char) * (bufferSize * 2));
        l->loadFirstPart = true;
    }

    return l;
}

Token lexer_getNextToken(Lexer *l) {
    Token t;
    t.type = S_OPEN_PARENTHESIS;

    // TODO: 

    return t;
}

void lexer_free(Lexer* l) {
    fclose(l->sourceFile);
    free(l->buffer);
    free(l);
}
