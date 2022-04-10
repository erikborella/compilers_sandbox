#ifndef RESPONSE_CREATOR_H
#define RESPONSE_CREATOR_H

#include <stdint.h>

enum content_type {
    TYPE_HTML,
    TYPE_JSON,
};

typedef struct responseCreator ResponseCreator;

ResponseCreator* responseCreator_init(enum content_type contentType, 
                                      uint16_t statusCode);
void responseCreator_free(ResponseCreator* rc);

void responseCreator_appendContent(ResponseCreator *rc, char *str);

char* responseCreator_getResponse(ResponseCreator *rc);

#endif