#include "responseCreator.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

struct content {
    char* str;
    struct content *next;
};

struct responseCreator {
    uint16_t statusCode;
    enum content_type contentType;
    struct content *head;
    size_t contentSize;
};

void* RC_mallocOrExitWithError(size_t size) {
    void* m = malloc(size);

    if (m == NULL) {
        fprintf(stderr, "Response Creator Error: Unable to allocate %lu bytes\n", size);
        exit(1);
    }

    return m;
}

ResponseCreator* responseCreator_init(enum content_type contentType, 
                                      uint16_t statusCode) {
    
    ResponseCreator *rc = malloc(sizeof(ResponseCreator));

    if (rc != NULL) {
        rc->contentType = contentType;
        rc->statusCode = statusCode;
        rc->contentSize = 0;
        rc->head = NULL;
    }

    return rc;
}

void responseCreator_free(ResponseCreator* rc) {
    struct content *no;

    while (rc->head != NULL) {
        no = rc->head;
        rc->head = rc->head->next;

        free(no->str);
        free(no);
    }

    free(rc);
}

void responseCreator_appendContent(ResponseCreator *rc, char *str) {
    struct content *no = RC_mallocOrExitWithError(sizeof(struct content));
    
    const size_t strSize = strlen(str);

    char *strCopy = RC_mallocOrExitWithError(sizeof(char) * strSize + 1);
    strcpy(strCopy, str);

    rc->contentSize += strSize;

    no->str = strCopy;
    no->next = NULL;

    if (rc->head == NULL) {
        rc->head = no;
    }
    else {
        struct content *aux = rc->head;

        while (aux->next != NULL)
            aux = aux->next;

        aux->next = no;
    }
}

char* responseCreator_getResponse(ResponseCreator *rc) {
    const char *headerTemplate = 
        "HTTP/1.1 %u %s\r\n"
        "Server: Integrated Compiler Server\r\n"
        "Content-Type: %s\r\n"
        "Connection: Closed\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "\r\n";

    const char *contentType;
    if (rc->contentType == TYPE_HTML)
        contentType = "text/html";
    else if (rc->contentType == TYPE_JSON)
        contentType = "application/json";
    else
        contentType = "plain/text";

    const char* statusCodeInfo;
    if (rc->statusCode == 404)
        statusCodeInfo = "NOT FOUND";
    else
        statusCodeInfo = "OK";

    char headerStr[255];
    sprintf(headerStr, headerTemplate, rc->statusCode, statusCodeInfo, contentType);

    char *contentStr = RC_mallocOrExitWithError(sizeof(char) * rc->contentSize + 3);
    bzero(contentStr, sizeof(char) * rc->contentSize + 3);

    struct content *no = rc->head;
    
    while (no != NULL) {
        strcat(contentStr, no->str);
        no = no->next;
    }

    strcat(contentStr, "\r\n");

    const size_t responseSize = strlen(headerStr) + rc->contentSize + 1;
    char *responseStr = RC_mallocOrExitWithError(sizeof(char) * responseSize);
    bzero(responseStr, sizeof(char) * responseSize);

    strncpy(responseStr, headerStr, strlen(headerStr));
    strcat(responseStr, contentStr);

    free(contentStr);

    return responseStr;
}