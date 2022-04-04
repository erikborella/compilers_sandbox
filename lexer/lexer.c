#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include "bufferReader/bufferReader.h"
#include "../symbolsTable/symbolsTable.h"
const struct LX_s_reservedWords {
    char* str;
    enum tokenType type;
} LX_reservedWords[] = {
    {.str = "void", .type = R_VOID},
    {.str = "main", .type = R_MAIN},
    {.str = "if", .type = R_IF},
    {.str = "else", .type = R_ELSE},
    {.str = "for", .type = R_FOR},
    {.str = "while", .type = R_WHILE},
    {.str = "int", .type = R_INT},
    {.str = "float", .type = R_FLOAT},
    {.str = "char", .type = R_CHAR},
    {.str = "scanf", .type = R_SCANF},
    {.str = "print", .type = R_PRINT},
    {.str = "return", .type = R_RETURN},
};

const size_t LX_sizeReservedWords = 
    sizeof(LX_reservedWords) / sizeof(struct LX_s_reservedWords);

struct lexer {
    BufferReader* bufferReader;
    SymbolsTable* symbolsTable;  
};

void LX_throwError(Lexer *l, const char* msg, ...) {
    bufferReader_ignoreSelected(l->bufferReader);
    FilePosition errorPosition = bufferReader_getPosition(l->bufferReader);

    fprintf(stderr, "Lexer Error -> L:%ld C:%ld: ",
        errorPosition.line, errorPosition.column);

    va_list arg_ptr;

    va_start(arg_ptr, msg);
    vfprintf(stderr, msg, arg_ptr);
    va_end(arg_ptr);

    exit(1);
}

#pragma region NUMBER

void LX_moveWhileIsNumber(Lexer* l) {
    while (!bufferReader_isEOF(l->bufferReader) && isdigit(bufferReader_getCurrent(l->bufferReader))) {
        bufferReader_moveNext(l->bufferReader);
    }
    
    const char current = bufferReader_getCurrent(l->bufferReader);

    if (isalpha(current))
        LX_throwError(l, "Expected a number or '.', but found a letter '%c'\n", current);
}

Token LX_getFloatNumber(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    char current = bufferReader_getCurrent(l->bufferReader);

    if (!isdigit(current))
        LX_throwError(l, "Expected a number after '.', but found '%c'\n", current);

    LX_moveWhileIsNumber(l);

    FilePosition position = bufferReader_getPosition(l->bufferReader);
    char *str = bufferReader_getSelected(l->bufferReader);
    
    Token t = {
        .type = V_NUM_FLOAT,
        .attribute.FLOAT_ATTR = strtod(str, NULL),
        .position = position
    };

    free(str);
    return t;
}

Token LX_getIntNumber(Lexer *l) {
    FilePosition position = bufferReader_getPosition(l->bufferReader);
    char *str = bufferReader_getSelected(l->bufferReader);

    Token t = {
        .type = V_NUM_INT,
        .attribute.INT_ATTR = strtoll(str, NULL, 10),
        .position = position
    };

    free(str);
    return t;
}

Token LX_getNumber(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    LX_moveWhileIsNumber(l);

    if (bufferReader_getCurrent(l->bufferReader) == '.')
        return LX_getFloatNumber(l);
    else {
        return LX_getIntNumber(l);
    }
}

#pragma endregion

#pragma region NAME

enum tokenType LX_getNameType(char* str) {
    for (int i = 0; i < LX_sizeReservedWords; i++) {
        const struct LX_s_reservedWords reservedWord = LX_reservedWords[i];

        if (strcmp(reservedWord.str, str) == 0)
            return reservedWord.type;
    }

    return I_ID;
}

Token LX_getName(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    while (!bufferReader_isEOF(l->bufferReader) && 
            (isalnum(bufferReader_getCurrent(l->bufferReader)) || bufferReader_getCurrent(l->bufferReader) == '_')) {
        bufferReader_moveNext(l->bufferReader);
    }

    FilePosition position = bufferReader_getPosition(l->bufferReader);
    char *str = bufferReader_getSelected(l->bufferReader);

    Token t;
    t.type = LX_getNameType(str);
    t.position = position;
    
    if (t.type == I_ID)
        t.attribute.INT_ATTR = symbolsTable_getIdOrAddSymbol(l->symbolsTable, str);
    else
        t.attribute.INT_ATTR = 0;

    free(str);

    return t;
}

#pragma endregion

Token LX_getString(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);
    bufferReader_ignoreSelected(l->bufferReader);

    while (!bufferReader_isEOF(l->bufferReader) && 
            bufferReader_getCurrent(l->bufferReader) != '\"') {
        bufferReader_moveNext(l->bufferReader);
    }

    FilePosition position = bufferReader_getPosition(l->bufferReader);
    char *str = bufferReader_getSelected(l->bufferReader);

    Token t;
    t.type = V_STRING;
    t.attribute.INT_ATTR = symbolsTable_getIdOrAddSymbol(l->symbolsTable, str);
    t.position = position;

    bufferReader_moveNext(l->bufferReader);
    bufferReader_ignoreSelected(l->bufferReader);

    free(str);

    return t;
}

Token LX_getSymbol(Lexer *l) {
    char symbol = bufferReader_getCurrent(l->bufferReader);

    bufferReader_moveNext(l->bufferReader);

    Token t;
    t.attribute.INT_ATTR = 0;

    switch (symbol) {
        case '(':
            t.type = S_OPEN_PARENTHESIS;
            break;
        case ')':
            t.type = S_CLOSE_PARENTHESIS;
            break;
        case '[':
            t.type = S_OPEN_SQUARE_BRACKETS;
            break;
        case ']':
            t.type = S_CLOSE_SQUARE_BRACKETS;
            break;
        case '{':
            t.type = S_OPEN_CURLY_BRACKETS;
            break;
        case '}':
            t.type = S_CLOSE_CURLY_BRACKETS;
            break;
        case ',':
            t.type = S_COMMA;
            break;
        case ';':
            t.type = S_SEMICOLON;
            break;
        case '.':
            t.type = S_DOT;
            break;

        default:
            fprintf(stderr, "Lexer Error: Invalid symbol: %c\n", symbol);
            exit(1);
            break;
    }

    FilePosition position = bufferReader_getPosition(l->bufferReader);
    t.position = position;

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma region PLUS

Token LX_getPlusToken(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);
    FilePosition position = bufferReader_getPosition(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
        .position = position
    };

    if (current == '+') {
        bufferReader_moveNext(l->bufferReader);
        t.type = O_INCREMENT;
    }
    else
        t.type = O_ADD;

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region MINUS

Token LX_getMinusToken(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);
    FilePosition position = bufferReader_getPosition(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
        .position = position
    };

    if (current == '-') {
        bufferReader_moveNext(l->bufferReader);
        t.type = O_DECREMENT;
    }
    else
        t.type = O_SUBTRACT;

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region ASTERISK

Token LX_getAsteriskToken(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);
    FilePosition position = bufferReader_getPosition(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
        .position = position
    };

    t.type = O_MULTIPLY;

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region SLASH

Token LX_getLineComment(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    while (!bufferReader_isEOF(l->bufferReader) && bufferReader_getCurrent(l->bufferReader) != '\n')
        bufferReader_moveNext(l->bufferReader);

    bufferReader_moveNext(l->bufferReader);
    
    FilePosition position = bufferReader_getPosition(l->bufferReader);
    bufferReader_ignoreSelected(l->bufferReader);

    Token t = {
        .type = C_LINE_COMMENT,
        .attribute.INT_ATTR = 0,
        .position = position
    };

    return t;
}

Token LX_getBlockComment(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    while (!bufferReader_isEOF(l->bufferReader)) {
        bufferReader_moveNext(l->bufferReader);

        const char current = bufferReader_getCurrent(l->bufferReader);

        if (current == '*') {
            bufferReader_moveNext(l->bufferReader);
            const char next = bufferReader_getCurrent(l->bufferReader);

            if (bufferReader_isEOF(l->bufferReader) || next == '/') {
                bufferReader_moveNext(l->bufferReader);
                break;
            }
        }
    }

    FilePosition position = bufferReader_getPosition(l->bufferReader);
    bufferReader_ignoreSelected(l->bufferReader);

    Token t = {
        .type = C_BLOCK_COMMENT,
        .attribute.INT_ATTR = 0,
        .position = position,
    };

    return t;
}

Token LX_getDivideOperator(Lexer *l) {
    FilePosition position = bufferReader_getPosition(l->bufferReader);
    bufferReader_ignoreSelected(l->bufferReader);


    Token t = {
        .type = O_DIVIDE,
        .attribute.INT_ATTR = 0,
        .position = position,
    };

    return t;
}

Token LX_getSlashToken(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);

    if (current == '/')
        return LX_getLineComment(l);
    else if (current == '*')
        return LX_getBlockComment(l);
    else
        return LX_getDivideOperator(l);
}

#pragma endregion

#pragma region EQUAL

Token LX_getEqualToken(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);
    FilePosition position = bufferReader_getPosition(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
        .position = position
    };

    if (current == '=') {
        t.type = O_EQUAL;
        bufferReader_moveNext(l->bufferReader);
    } 
    else
        t.type = S_ATTRIBUTION;

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region GREATER

Token LX_getGreaterToken(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);
    FilePosition position = bufferReader_getPosition(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
        .position = position
    };

    if (current == '=') {
        t.type = O_GREATER_EQUAL;
        bufferReader_moveNext(l->bufferReader);
    } 
    else
        t.type = O_GREATER;

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region LESS

Token LX_getLessToken(Lexer *l) {
     bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);
    FilePosition position = bufferReader_getPosition(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
        .position = position
    };

    if (current == '=') {
        t.type = O_LESS_EQUAL;
        bufferReader_moveNext(l->bufferReader);
    } 
    else
        t.type = O_LESS;

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region TAD METHODS

Lexer* lexer_init(const char* sourceFilePath, size_t bufferSize, SymbolsTable* symbolsTable) {
    Lexer* l = (Lexer*) malloc(sizeof(Lexer));

    if (l != NULL) {
        l->bufferReader = bufferReader_init(sourceFilePath, bufferSize);
        l->symbolsTable = symbolsTable;
    }

    return l;
}

Token lexer_getNextToken(Lexer *l) {
    Token t;
    bool tokenFound;
    
    do {
        tokenFound = true;
        char current = bufferReader_getCurrent(l->bufferReader);

        if (isdigit(current)) {
            t = LX_getNumber(l);
        }
        else if (isalpha(current)) {
            t = LX_getName(l);
        }
        else if (current == '\"') {
            t = LX_getString(l);
        }
        else if (current == '+') {
            t = LX_getPlusToken(l);
        }
        else if (current == '-') {
            t = LX_getMinusToken(l);
        }
        else if (current == '*') {
            t = LX_getAsteriskToken(l);
        }
        else if (current == '/') {
            t = LX_getSlashToken(l);
        }
        else if (current == '=') {
            t = LX_getEqualToken(l);
        }
        else if (current == '>') {
            t = LX_getGreaterToken(l);
        }
        else if (current == '<') {
            t = LX_getLessToken(l);
        }
        else if (ispunct(current)) {
            t = LX_getSymbol(l);
        }
        else {
            bufferReader_moveNext(l->bufferReader);
            bufferReader_ignoreSelected(l->bufferReader);

            tokenFound = false;
        }
    } while (!tokenFound);
    

    return t;
}

bool lexer_hasNext(Lexer *l) {
    while (!bufferReader_isEOF(l->bufferReader) && bufferReader_getCurrent(l->bufferReader) == '\n') {
        bufferReader_moveNext(l->bufferReader);
    }

    bufferReader_ignoreSelected(l->bufferReader);

    return !bufferReader_isEOF(l->bufferReader);
}

void lexer_free(Lexer* l) {
    bufferReader_free(l->bufferReader);
    free(l);
}

#pragma endregion