#include <stdio.h>
#include <stdlib.h>

#include "lexer/lexer.h"
#include "symbolsTable/symbolsTable.h"

#define CODE_SOURCE_FILE "code_example.txt"

const char* getTokenTypeAsString(enum tokenType type) {
    switch (type) {
        case I_ID:
            return "I_ID";
        case V_NUM_INT:
            return "V_NUM_INT";
        case V_NUM_FLOAT:
            return "V_NUM_FLOAT";
        case V_STRING:
            return "V_STRING";
        case R_VOID:
            return "R_VOID";
        case R_MAIN:
            return "R_MAIN";
        case R_IF:
            return "R_IF";
        case R_ELSE:
            return "R_ELSE";
        case R_FOR:
            return "R_FOR";
        case R_WHILE:
            return "R_WHILE";
        case R_INT:
            return "R_INT";
        case R_FLOAT:
            return "R_FLOAT";
        case R_CHAR:
            return "R_CHAR";
        case R_SCANF:
            return "R_SCANF";
        case R_PRINT:
            return "R_PRINT";
        case R_RETURN:
            return "R_RETURN";
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
        case O_MOD:
            return "O_MOD";
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
        case C_LINE_COMMENT:
            return "C_LINE_COMMENT";
        case C_BLOCK_COMMENT:
            return "C_BLOCK_COMMENT";
    }
}

void printToken(Token t) {
    if (t.type == V_NUM_FLOAT) {
        printf("Token => type: %s\n\tposition: L:%ld C:%ld\n\tattr: %f\n\n",
            getTokenTypeAsString(t.type),
            t.position.line, t.position.column,
            t.attribute.FLOAT_ATTR);
    }
    else {
        printf("Token => type: %s\n\tposition: L:%ld C:%ld\n\tattr: %d\n\n",
            getTokenTypeAsString(t.type),
            t.position.line, t.position.column,
            t.attribute.INT_ATTR);
    }
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