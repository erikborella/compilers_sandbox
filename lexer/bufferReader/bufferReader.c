#include "bufferReader.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

struct bufferReader {
    FILE* sourceFile;
    size_t bufferSize;
    char* buffer;
    bool loadFirstPart;
    size_t startPtr;
    size_t endPtr;
    FilePosition startPosition;
    FilePosition endPosition;
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
        exit(1);
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

size_t BR_mod(size_t a, size_t b) {
    size_t r = a % b;
    
    if (r < 0)
        return r + b;
    else
        return r;
}

void BR_finishSelection(BufferReader* br) {
    br->startPtr = br->endPtr;
    br->startPosition = br->endPosition;
}

void BR_updateEndPosition(BufferReader* br, char ch) {
    if (ch == '\n') {
        br->endPosition.line++;
        br->endPosition.column = 0;
    }
    else {
        br->endPosition.column++;
    }
}

BufferReader* bufferReader_init(const char* sourceFilePath, size_t bufferSize) {
    BufferReader* br = (BufferReader*) malloc(sizeof(BufferReader));

    if (br != NULL) {
        br->sourceFile = BR_openFileAsReadOrExitWithError(sourceFilePath);
        br->bufferSize = bufferSize;

        //Double the buffer size to use the double buffer technique
        br->buffer = BR_mallocOrExitWithError(sizeof(char) * (bufferSize * 2));
        memset(br->buffer, 0, sizeof(char) * (bufferSize * 2));

        br->loadFirstPart = true;
        br->startPtr = 0;
        br->endPtr = 0;

        FilePosition startFile =  {.line = 1, .column = 1};
        br->startPosition = startFile;
        br->endPosition = startFile;

        BR_loadChunk(br);
    }

    return br;
}

void bufferReader_free(BufferReader* br) {
    fclose(br->sourceFile);
    free(br->buffer);
    free(br);
}

bool bufferReader_isEOF(BufferReader* br) {
    return br->buffer[br->endPtr] == 0;
}

void bufferReader_moveNext(BufferReader* br) {
    br->endPtr = BR_mod(br->endPtr + 1, br->bufferSize * 2);

    char current = bufferReader_getCurrent(br);
    BR_updateEndPosition(br, current);

    if (br->endPtr == 0 || br->endPtr == br->bufferSize)
        BR_loadChunk(br);
}

char bufferReader_getCurrent(BufferReader* br) {
    return br->buffer[br->endPtr];
}

char* bufferReader_getSelected(BufferReader* br) {
    size_t selectedLen;
    char* selected;

    if (br->endPtr > br->startPtr)
        selectedLen = br->endPtr - br->startPtr;
    else
        selectedLen = br->endPtr - br->startPtr + (br->bufferSize * 2);

    selected = BR_mallocOrExitWithError(sizeof(char) * (selectedLen + 1));

    int i = br->startPtr;
    int j = 0;

    while (i != br->endPtr) {
        selected[j] = br->buffer[i];
        
        i = BR_mod(i + 1, br->bufferSize * 2);
        j++;
    }

    selected[selectedLen] = 0;

    BR_finishSelection(br);

    return selected;
}

void bufferReader_ignoreSelected(BufferReader* br) {
    BR_finishSelection(br);
}

FilePosition bufferReader_getPosition(BufferReader* br) {
    return br->startPosition;
}