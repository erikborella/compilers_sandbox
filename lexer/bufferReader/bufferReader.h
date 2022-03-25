#ifndef BUFFER_READER_H
#define BUFFER_READER_H

#include <stddef.h>
#include <stdbool.h>

typedef struct bufferReader BufferReader;

BufferReader* bufferReader_init(const char* sourceFilePath, size_t bufferSize);
void bufferReader_free(BufferReader* br);

bool bufferReader_isEOF(BufferReader* br);
char bufferReader_getNext(BufferReader* br);

#endif