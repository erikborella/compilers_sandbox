#include "bufferReader.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

struct bufferReader {
    FILE* sourceFile;
    size_t bufferSize;
    char* buffer;
    bool loadFirstPart;
};

FILE* BR_openFileAsReadOrExitWithError(const char* sourceFilePath) {
    FILE* f = fopen(sourceFilePath, "r");

    if (f == NULL) {
        fprintf(stderr, "Lexer Error: Unable to open file \"%s\"\n", sourceFilePath);
        exit(1);
    }

    return f;
}

void* BR_mallocOrExitWithError(size_t size) {
    void* m = malloc(size);

    if (m == NULL) {
        fprintf(stderr, "Lexer Error: Unable to allocate %lu bytes\n", size);
    }

    return m;
}

void BR_loadChunk(BufferReader* br) {
    size_t startPositionToLoad;
    size_t bytesRead;

    if (br->loadFirstPart)
        startPositionToLoad = 0;
    else
        startPositionToLoad = br->bufferSize;

    bytesRead = fread(br->buffer + startPositionToLoad, sizeof(char), br->bufferSize, br->sourceFile);

    if (bytesRead < br->bufferSize)
        *(br->buffer + startPositionToLoad + bytesRead) = 0;

    br->loadFirstPart = !br->loadFirstPart;
}

BufferReader* bufferReader_init(const char* sourceFilePath, size_t bufferSize) {
    BufferReader* br = (BufferReader*) malloc(sizeof(BufferReader));

    if (br != NULL) {
        br->sourceFile = BR_openFileAsReadOrExitWithError(sourceFilePath);
        br->bufferSize = bufferSize;
        //Double the buffer size to use the double buffer technique
        br->buffer = BR_mallocOrExitWithError(sizeof(char) * (bufferSize * 2));
        br->loadFirstPart = true;
    }

    return br;
}

void bufferReader_free(BufferReader* br) {
    fclose(br->sourceFile);
    free(br->buffer);
    free(br);
}