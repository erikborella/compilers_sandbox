#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "bufferReader/bufferReader.h"
#include "../symbolsTable/symbolsTable.h"

const struct LX_s_reservedWords {
    char* str;
    enum tokenType type;
} LX_reservedWords[] = {
    {.str = "public", .type = R_PUBLIC},
    {.str = "static", .type = R_STATIC},
    {.str = "void", .type = R_VOID},
    {.str = "int", .type = R_INT},
    {.str = "if", .type = R_IF},
    {.str = "else", .type = R_ELSE},
    {.str = "for", .type = R_FOR},
};

const size_t LX_sizeReservedWords = 
    sizeof(LX_reservedWords) / sizeof(struct LX_s_reservedWords);

struct lexer {
    BufferReader* bufferReader;
    SymbolsTable* symbolsTable;  
};

Token LX_getNumber(Lexer *l) {
    bufferReader_moveNext(l->bufferReader);

    while (!bufferReader_isEOF(l->bufferReader) && isdigit(bufferReader_getCurrent(l->bufferReader))) {
        bufferReader_moveNext(l->bufferReader);
    }

    FilePosition position = bufferReader_getPosition(l->bufferReader);
    char *str = bufferReader_getSelected(l->bufferReader);

    Token t;
    t.type = NUMBER;
    t.attribute = strtoll(str, NULL, 10);
    t.position = position;

    return t;
}

enum tokenType LX_getNameType(char* str) {
    for (int i = 0; i < LX_sizeReservedWords; i++) {
        const struct LX_s_reservedWords reservedWord = LX_reservedWords[i];
        
        if (strcmp(reservedWord.str, str) == 0)
            return reservedWord.type;
    }

    return ID;
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
    
    if (t.type == ID)
        t.attribute = symbolsTable_getIdOrAddSymbol(l->symbolsTable, str);
    else
        t.attribute = 0;

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
    t.type = STRING;
    t.attribute = symbolsTable_getIdOrAddSymbol(l->symbolsTable, str);
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
    t.attribute = 0;

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
    t.attribute = 0;

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
    t.attribute = 0;

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
    return !bufferReader_isEOF(l->bufferReader);
}

void lexer_free(Lexer* l) {
    bufferReader_free(l->bufferReader);
    free(l);
}
