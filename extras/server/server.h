#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>
#include "responseCreator/responseCreator.h"

enum http_method {
    HTTP_GET,
    HTTP_POST,
    HTTP_OTHER,
};

typedef struct {
    enum http_method method;
    char *path;
    char *content;
} Request;

typedef ResponseCreator* (*RouteCallback)(Request req);

typedef struct server Server;

Server* server_init(uint16_t port);
void server_free(Server *s);

void server_addRoute(Server *s, const char *path, 
                     enum http_method method, RouteCallback callback);

void server_start(Server *s);

#endif