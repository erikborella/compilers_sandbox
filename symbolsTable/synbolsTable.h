#ifndef SYMBOLS_TABLE_H
#define SYMBOLS_TABLE_H

#include <stddef.h>

typedef struct symbolsTable SymbolsTable;

SymbolsTable* symbolsTable_init();
void symbolsTable_free(SymbolsTable* st);

size_t symbolsTable_getIdOrAddSymbol(SymbolsTable* st, char* symbolName);

#endif