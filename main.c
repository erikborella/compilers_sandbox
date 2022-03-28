#include <stdio.h>
#include <stdlib.h>

#include "lexer/lexer.h"
#include "symbolsTable/symbolsTable.h"

#define CODE_SOURCE_FILE "code_example.txt"

const char* getTokenTypeAsString(enum tokenType type) {
    switch (type) {
        case ID:
            return "ID";
        case NUMBER:
            return "NUMBER";
        case STRING:
            return "STRING";
        case R_PUBLIC:
            return "R_PUBLIC";
        case R_STATIC:
            return "R_STATIC";
        case R_VOID:
            return "R_VOID";
        case R_INT:
            return "R_INT";
        case R_IF:
            return "R_IF";
        case R_ELSE:
            return "R_ELSE";
        case R_FOR:
            return "R_FOR";
        case S_OPEN_PARENTHESIS:
            return "S_OPEN_PARENTHESIS";
        case S_CLOSE_PARENTHESIS:
            return "S_CLOSE_PARENTHESIS";
        case S_OPEN_SQUARE_BRACKETS:
            return "S_OPEN_SQUARE_BRACKETS";
        case S_CLOSE_SQUARE_BRACKETS:
            return "S_CLOSE_SQUARE_BRACKETS";
        case S_OPEN_CURLY_BRACKETS:
            return "S_OPEN_CURLY_BRACKETS";
        case S_CLOSE_CURLY_BRACKETS:
            return "S_CLOSE_CURLY_BRACKETS";
        case S_ATTRIBUTION:
            return "S_ATTRIBUTION";
        case S_COMMA:
            return "S_COMMA";
        case S_SEMICOLON:
            return "S_SEMICOLON";
        case S_DOT:
            return "S_DOT";
        case O_EQUAL:
            return "O_EQUAL";
        case O_ADD:
            return "O_ADD";
        case O_SUBTRACT:
            return "O_SUBTRACT";
        case O_MULTIPLY:
            return "O_MULTIPLY";
        case O_DIVIDE:
            return "O_DIVIDE";
        case O_LESS:
            return "O_LESS";
        case O_LESS_EQUAL:
            return "O_LESS_EQUAL";
        case O_GREATER:
            return "O_GREATER";
        case O_GREATER_EQUAL:
            return "O_GREATER_EQUAL";
        case O_INCREMENT:
            return "O_INCREMENT";
        case O_DECREMENT:
            return "O_DECREMENT";
    }
}

void printToken(Token t) {
    printf("Token => type: %s - attr: %d\n", getTokenTypeAsString(t.type), t.attribute);
}

int main() {

    SymbolsTable* st = symbolsTable_init();

    Lexer* l = lexer_init(CODE_SOURCE_FILE, 100, st);

    while (lexer_hasNext(l)) {
        Token t = lexer_getNextToken(l);
        printToken(t);
    }

    lexer_free(l);

    symbolsTable_free(st);

    return 0;
}