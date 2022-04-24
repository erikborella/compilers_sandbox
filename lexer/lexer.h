#ifndef LEXER_H
#define LEXER_H

#include "../symbolsTable/symbolsTable.h"

#include <stddef.h>
#include <stdbool.h>

#include "bufferReader/bufferReader.h"

typedef struct lexer Lexer;

/*
I: Identifier
V: Value Type
R: Reserved
S: Symbol
O: Operator
C: Commentary
*/
enum tokenType {
    I_ID,
    V_NUM_INT,
    V_NUM_FLOAT,
    V_STRING,
    R_VOID,
    R_MAIN,
    R_IF,
    R_ELSE,
    R_FOR,
    R_WHILE,
    R_INT,
    R_FLOAT,
    R_CHAR,
    R_SCANF,
    R_PRINT,
    R_RETURN,
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
    O_MOD,
    O_LESS,
    O_LESS_EQUAL,
    O_GREATER,
    O_GREATER_EQUAL,
    O_INCREMENT,
    O_DECREMENT,
    C_LINE_COMMENT,
    C_BLOCK_COMMENT,
};

typedef struct {
    enum tokenType type;
    FileLocation location;
    union {
        int INT_ATTR;
        double FLOAT_ATTR;  
    } attribute;
    
} Token;


Lexer* lexer_init(const char* sourceFilePath, size_t bufferSize, SymbolsTable* symbolsTable);
void lexer_free(Lexer* l);

Token lexer_getNextToken(Lexer *l);
bool lexer_hasNext(Lexer *l);

#endif