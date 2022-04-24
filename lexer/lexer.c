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
    FileLocation errorPosition = bufferReader_getLocation(l->bufferReader);

    fprintf(stderr, "Lexer Error -> L:%ld C:%ld: ",
        errorPosition.start.line, errorPosition.start.column);

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

    FileLocation location = bufferReader_getLocation(l->bufferReader);
    char *str = bufferReader_getSelected(l->bufferReader);
    
    Token t = {
        .type = V_NUM_FLOAT,
        .attribute.FLOAT_ATTR = strtod(str, NULL),
        .location = location
    };

    free(str);
    return t;
}

Token LX_getIntNumber(Lexer *l) {
    FileLocation location = bufferReader_getLocation(l->bufferReader);
    char *str = bufferReader_getSelected(l->bufferReader);

    Token t = {
        .type = V_NUM_INT,
        .attribute.INT_ATTR = strtoll(str, NULL, 10),
        .location = location
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

    FileLocation location = bufferReader_getLocation(l->bufferReader);
    char *str = bufferReader_getSelected(l->bufferReader);

    Token t = {
        .type = LX_getNameType(str),
        .location = location,
    };
    
    if (t.type == I_ID)
        t.attribute.INT_ATTR = symbolsTable_getIdOrAddSymbol(l->symbolsTable, str);
    else
        t.attribute.INT_ATTR = 0;

    free(str);

    return t;
}

#pragma endregion

#pragma region STRING

Token LX_getString(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    while (!bufferReader_isEOF(l->bufferReader)) {
        const char current = bufferReader_getCurrent(l->bufferReader);

        if (current == '\n')
            LX_throwError(l, "Multi-line strings are not allowed\n");
        else if (current == '\"')
            break;

        bufferReader_moveNext(l->bufferReader);
    }

    bufferReader_moveNext(l->bufferReader);
    FileLocation location = bufferReader_getLocation(l->bufferReader);
    char *str = bufferReader_getSelected(l->bufferReader);

    Token t = {
        .type = V_STRING,
        .attribute.INT_ATTR = symbolsTable_getIdOrAddSymbol(l->symbolsTable, str),
        .location = location,
    };

    free(str);

    return t;
}

#pragma endregion

#pragma region SYMBOLS

Token LX_getSymbol(Lexer *l) {
    char current = bufferReader_getCurrent(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0
    };

    switch (current) {
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
            LX_throwError(l, "Invalid symbol: %c\n", current);
    }

    bufferReader_moveNext(l->bufferReader);

    FileLocation location = bufferReader_getLocation(l->bufferReader);
    t.location = location;

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region PLUS

Token LX_getPlusToken(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
    };

    if (current == '+') {
        bufferReader_moveNext(l->bufferReader);
        t.type = O_INCREMENT;
    }
    else
        t.type = O_ADD;

    t.location = bufferReader_getLocation(l->bufferReader);

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region MINUS

Token LX_getMinusToken(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
    };

    if (current == '-') {
        bufferReader_moveNext(l->bufferReader);
        t.type = O_DECREMENT;
    }
    else
        t.type = O_SUBTRACT;

    t.location = bufferReader_getLocation(l->bufferReader);

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region ASTERISK

Token LX_getAsteriskToken(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);
    FileLocation location = bufferReader_getLocation(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
        .location = location,
        .type = O_MULTIPLY,
    };

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region SLASH

Token LX_getLineComment(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    while (!bufferReader_isEOF(l->bufferReader) && bufferReader_getCurrent(l->bufferReader) != '\n')
        bufferReader_moveNext(l->bufferReader);

    FileLocation location = bufferReader_getLocation(l->bufferReader);

    bufferReader_moveNext(l->bufferReader);
    bufferReader_ignoreSelected(l->bufferReader);

    Token t = {
        .type = C_LINE_COMMENT,
        .attribute.INT_ATTR = 0,
        .location = location
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

    FileLocation location = bufferReader_getLocation(l->bufferReader);
    bufferReader_ignoreSelected(l->bufferReader);

    Token t = {
        .type = C_BLOCK_COMMENT,
        .attribute.INT_ATTR = 0,
        .location = location,
    };

    return t;
}

Token LX_getDivideOperator(Lexer *l) {
    FileLocation location = bufferReader_getLocation(l->bufferReader);
    bufferReader_ignoreSelected(l->bufferReader);


    Token t = {
        .type = O_DIVIDE,
        .attribute.INT_ATTR = 0,
        .location = location,
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

#pragma region PERCENTAGE

Token LX_getPercentageToken(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);
    FileLocation location = bufferReader_getLocation(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
        .location = location,
        .type = O_MOD,
    };

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region EQUAL

Token LX_getEqualToken(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
    };

    if (current == '=') {
        t.type = O_EQUAL;
        bufferReader_moveNext(l->bufferReader);
    } 
    else
        t.type = S_ATTRIBUTION;

    t.location = bufferReader_getLocation(l->bufferReader);

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region GREATER

Token LX_getGreaterToken(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
    };

    if (current == '=') {
        t.type = O_GREATER_EQUAL;
        bufferReader_moveNext(l->bufferReader);
    } 
    else
        t.type = O_GREATER;

    t.location = bufferReader_getLocation(l->bufferReader);

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

#pragma endregion

#pragma region LESS

Token LX_getLessToken(Lexer *l) {
     bufferReader_moveNext(l->bufferReader);

    const char current = bufferReader_getCurrent(l->bufferReader);

    Token t = {
        .attribute.INT_ATTR = 0,
    };

    if (current == '=') {
        t.type = O_LESS_EQUAL;
        bufferReader_moveNext(l->bufferReader);
    } 
    else
        t.type = O_LESS;

    t.location = bufferReader_getLocation(l->bufferReader);

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

        switch (current) {
            case '\"':
                t = LX_getString(l);
                break;
                
            case '+':
                t = LX_getPlusToken(l);
                break;
                
            case '-':
                t = LX_getMinusToken(l);
                break;
                
            case '*':
                t = LX_getAsteriskToken(l);
                break;
                
            case '/':
                t = LX_getSlashToken(l);
                break;

            case '%':
                t = LX_getPercentageToken(l);
                break;
                
            case '=':
                t = LX_getEqualToken(l);
                break;
                
            case '>':
                t = LX_getGreaterToken(l);
                break;
                
            case '<':
                t = LX_getLessToken(l);
                break;
                
            default:
                if (isdigit(current))
                    t = LX_getNumber(l);
                else if (isalpha(current))
                    t = LX_getName(l);
                else if (ispunct(current))
                    t = LX_getSymbol(l);
                else {
                    bufferReader_moveNext(l->bufferReader);
                    bufferReader_ignoreSelected(l->bufferReader);
    
                    tokenFound = false;
                }
                break;
                
        }
    } while (!tokenFound);
    

    return t;
}

bool lexer_hasNext(Lexer *l) {
    char current = bufferReader_getCurrent(l->bufferReader);

    while (!bufferReader_isEOF(l->bufferReader) && isspace(current)) {
        bufferReader_moveNext(l->bufferReader);
        current = bufferReader_getCurrent(l->bufferReader);
    }

    bufferReader_ignoreSelected(l->bufferReader);

    return !bufferReader_isEOF(l->bufferReader);
}

void lexer_free(Lexer* l) {
    bufferReader_free(l->bufferReader);
    free(l);
}

#pragma endregion