#ifndef LEXER_H
#define LEXER_H

#include "../symbolsTable/symbolsTable.h"

#include <stddef.h>
#include <stdbool.h>

#include "bufferReader/bufferReader.h"

typedef struct lexer Lexer;

/*
R: Reserved
S: Symbol
O: Operator
*/
enum tokenType {
    ID,
    NUMBER,
    STRING,
    R_PUBLIC,
    R_STATIC,
    R_VOID,
    R_INT,
    R_IF,
    R_ELSE,
    R_FOR,
    S_OPEN_PARENTHESIS,
    S_CLOSE_PARENTHESIS,
    S_OPEN_SQUARE_BRACKETS,
    S_CLOSE_SQUARE_BRACKETS,
    S_OPEN_CURLY_BRACKETS,
    S_CLOSE_CURLY_BRACKETS,
    S_ATTRIBUTION,
    S_COMMA,
    S_SEMICOLON,
    S_DOT,
    O_EQUAL,
    O_ADD,
    O_SUBTRACT,
    O_MULTIPLY,
    O_DIVIDE,
    O_LESS,
    O_LESS_EQUAL,
    O_GREATER,
    O_GREATER_EQUAL,
    O_INCREMENT,
    O_DECREMENT,
};

typedef struct {
    enum tokenType type;
    int attribute;
    FilePosition position;
} Token;


Lexer* lexer_init(const char* sourceFilePath, size_t bufferSize, SymbolsTable* symbolsTable);
void lexer_free(Lexer* l);

Token lexer_getNextToken(Lexer *l);
bool lexer_hasNext(Lexer *l);

#endif