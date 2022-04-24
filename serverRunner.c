#include "extras/server/server.h"
#include "extras/server/responseCreator/responseCreator.h"

#include "symbolsTable/symbolsTable.h"
#include "lexer/lexer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

static volatile Server* serverReference = NULL;

const char* getTokenTypeAsString(enum tokenType type);

void intHandler(int num) {
    if (serverReference != NULL)
        server_free((Server*) serverReference);

    exit(num);
}

char* createTempCodeFile(const char *content) {
    char tempFilePath[] = "tempCode.XXXXXX";

    int fd = mkstemp(tempFilePath);
    FILE *tempFile = fdopen(fd, "w");

    if (fd == -1 || tempFile == NULL) {
        fprintf(stderr, "Runner Error => root: Unable to open temporary file\n");
        exit(1);
    }

    fwrite(content, strlen(content), 1, tempFile);

    fclose(tempFile);

    char *filePath = malloc(sizeof(char) * sizeof(tempFilePath));
    strcpy(filePath, tempFilePath);

    return filePath;
}

ResponseCreator* lexer(Request r) {
    const char *tokenJsonTemplate = 
        "{"
            "\"type\": \"%s\","
            "\"location\": {"
                "\"start\": {"
                    "\"line\": %d,"
                    "\"column\": %d"    
                "},"
                "\"end\": {"
                    "\"line\": %d,"
                    "\"column\": %d"    
                "}"
            "},"
            "\"attr\": %s"
        "}\0";

    char* tempFilePath = createTempCodeFile(r.content);
    char buff[255];

    SymbolsTable *st = symbolsTable_init();
    Lexer *l = lexer_init(tempFilePath, 1024, st);

    ResponseCreator* rc = responseCreator_init(TYPE_JSON, 200);

    responseCreator_appendContent(rc, "[");

    while (lexer_hasNext(l)) {
        Token t = lexer_getNextToken(l);

        char attrBuff[50];
        if (t.type == V_NUM_FLOAT)
            sprintf(attrBuff, "%f", t.attribute.FLOAT_ATTR);
        else
            sprintf(attrBuff, "%d", t.attribute.INT_ATTR);

        sprintf(buff, tokenJsonTemplate, getTokenTypeAsString(t.type), 
            t.location.start.line, t.location.start.column, 
            t.location.end.line, t.location.end.column,
            attrBuff);

        responseCreator_appendContent(rc, buff);

        if (lexer_hasNext(l))
            responseCreator_appendContent(rc, ",");
    }

    responseCreator_appendContent(rc, "]");

    lexer_free(l);
    symbolsTable_free(st);

    remove(tempFilePath);
    free(tempFilePath);

    return rc;
}

int main() {
    signal(SIGINT, intHandler);

    Server* s = server_init(8000);
    serverReference = s;

    server_addRoute(s, "/lexer", HTTP_POST, lexer);

    server_start(s);

    server_free(s);

    return 0;
}

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
