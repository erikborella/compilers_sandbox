#ifndef BUFFER_READER_H
#define BUFFER_READER_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    size_t line;
    size_t column;
} FilePosition;

typedef struct {
    FilePosition start;
    FilePosition end;
} FileLocation;

typedef struct bufferReader BufferReader;

BufferReader* bufferReader_init(const char* sourceFilePath, size_t bufferSize);
void bufferReader_free(BufferReader* br);

bool bufferReader_isEOF(BufferReader* br);
void bufferReader_moveNext(BufferReader* br);
char bufferReader_getCurrent(BufferReader* br);
char* bufferReader_getSelected(BufferReader* br);
void bufferReader_ignoreSelected(BufferReader* br);
FileLocation bufferReader_getLocation(BufferReader* br);

#endif