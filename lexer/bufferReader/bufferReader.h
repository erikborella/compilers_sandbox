#ifndef BUFFER_READER_H
#define BUFFER_READER_H

#include <stddef.h>

typedef struct bufferReader BufferReader;

BufferReader* bufferReader_init(const char* sourceFilePath, size_t bufferSize);
void bufferReader_free(BufferReader* br);

#endif