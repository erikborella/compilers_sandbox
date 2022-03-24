#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef struct lexer Lexer;

/*
R: Reserved
S: Symbol
O: Operator
*/
enum tokenType {
    ID,
    NUMBER,
    R_PUBLIC,
    R_STATIC,
    R_VOID,
    S_OPEN_PARENTHESIS,
    S_CLOSE_PARENTHESIS,
    S_OPEN_SQUARE_BRACKETS,
    S_CLOSE_SQUARE_BRACKETS,
    S_OPEN_CURLY_BRACKETS,
    S_CLOSE_CURLY_BRACKETS,
    S_EQUAL,
    S_COMMA,
    S_SEMICOLON,
    S_DOT,
    O_DIVIDE,
};

typedef struct {
    enum tokenType type;
    int attribute;
} Token;


Lexer* lexer_init(const char* sourceFilePath, size_t bufferSize);
void lexer_free(Lexer* l);

Token lexer_getNextToken(Lexer *l);

#endif