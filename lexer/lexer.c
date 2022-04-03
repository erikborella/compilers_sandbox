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

bool LX_isOperator(char ch) {
    return ch == '+' ||
           ch == '-' ||
           ch == '*' ||
           ch == '/' ||
           ch == '<' ||
           ch == '>';
}

Token LX_getOperator(Lexer *l) {
    char current = bufferReader_getCurrent(l->bufferReader);
    Token t;
    t.attribute.INT_ATTR = 0;

    bufferReader_moveNext(l->bufferReader);
    
    if ((current == '<' || current == '>') && bufferReader_getCurrent(l->bufferReader) == '=') {
        bufferReader_moveNext(l->bufferReader);

        switch (current) {
            case '<':
                t.type = O_LESS_EQUAL;
                break;
            case '>':
                t.type = O_GREATER_EQUAL;
                break;

            default:
                fprintf(stderr, "Lexer Error: This erro should never happen\n");
                exit(1);
                break;
        }
    }
    else if (current == '+' && bufferReader_getCurrent(l->bufferReader) == '+') {
        bufferReader_moveNext(l->bufferReader);
        t.type = O_INCREMENT;
    }
    else if (current == '-' && bufferReader_getCurrent(l->bufferReader) == '-') {
        bufferReader_moveNext(l->bufferReader);
        t.type = O_DECREMENT;
    }
    else {
        switch (current) {
            case '+':
                t.type = O_ADD;
                break;
            case '-':
                t.type = O_SUBTRACT;
                break;
            case '*':
                t.type = O_MULTIPLY;
                break;
            case '/':
                t.type = O_SUBTRACT;
                break;
            case '<':
                t.type = O_LESS;
                break;
            case '>':
                t.type = O_GREATER;
                break;

            default:
                fprintf(stderr, "Lexer Error: This erro should never happen\n");
                exit(1);
                break;
        }
    }

    FilePosition position = bufferReader_getPosition(l->bufferReader);
    t.position = position;

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

Token LX_getAttributionOrEqual(Lexer *l) {
    Token t;
    t.attribute.INT_ATTR = 0;

    bufferReader_moveNext(l->bufferReader);

    if (bufferReader_getCurrent(l->bufferReader) == '=') {
        t.type = O_EQUAL;
        bufferReader_moveNext(l->bufferReader);
    }
    else {
        t.type = S_ATTRIBUTION;
    }

    FilePosition position = bufferReader_getPosition(l->bufferReader);
    t.position = position;

    bufferReader_ignoreSelected(l->bufferReader);

    return t;
}

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
        else if (current == '=') {
            t = LX_getAttributionOrEqual(l);
        }
        else if (LX_isOperator(current)) {
            t = LX_getOperator(l);
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
