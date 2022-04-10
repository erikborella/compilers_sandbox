#include "server.h"

#include <stdio.h>
#include <stdint.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define REQUEST_MAX_SIZE 1000000
#define ROUTES_MAX_SIZE 2
#define SA struct sockaddr

typedef struct {
    const char* path;
    enum http_method method;
    RouteCallback callback;
} Route;

struct server {
    int sockfd;
    int connfd;
    Route routes[ROUTES_MAX_SIZE];
    size_t routesPtr;
    uint16_t port;
};

/*
    TCP sockets inspired by:
        - https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
        - https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpserver.c

    HTTP responses inspired by:
        - https://www.tutorialspoint.com/http/http_responses.htm
*/

enum http_method SV_getRequestHttpMethod(char request[REQUEST_MAX_SIZE], size_t *requestPtr) {
    size_t methodLen = 0;
    
    *requestPtr = 0;

    while (request[*requestPtr + methodLen] != ' ')
        methodLen++;

    const size_t mallocSize = sizeof(char) * methodLen + 1;
    char *strMethod = malloc(mallocSize);
    bzero(strMethod, mallocSize);

    strncpy(strMethod, request, methodLen);

    enum http_method method;

    if (strcmp("GET", strMethod) == 0)
        method = HTTP_GET;
    else if (strcmp("POST", strMethod) == 0)
        method = HTTP_POST;
    else
        method = HTTP_OTHER;

    free(strMethod);
    *requestPtr = methodLen + 1;

    return method;
}

char* SV_getRequestPath(char request[REQUEST_MAX_SIZE], size_t *requestPtr) {
    size_t routeLen = 0;

    while (request[*requestPtr + routeLen] != ' ')
        routeLen++;

    const size_t mallocSize = sizeof(char) * routeLen + 1;
    char *route = malloc(mallocSize);
    bzero(route, mallocSize);

    strncpy(route, request + *requestPtr, routeLen);

    return route;
}

char* SV_getRequestContent(char request[REQUEST_MAX_SIZE], size_t requestLen) {
    char *contentStart = strstr(request, "\r\n\r\n");
    contentStart = contentStart + 4;

    const size_t contentLen = requestLen - (contentStart - request);
    const size_t mallocSize = sizeof(char) * contentLen;

    char *content = malloc(sizeof(char) * contentLen);
    bzero(content, mallocSize);

    strcpy(content, contentStart);

    return content;
}

Request SV_parseRequest(Server *s) {
    char buff[REQUEST_MAX_SIZE];
    bzero(buff, REQUEST_MAX_SIZE);

    int connfd = s->connfd;

    ssize_t t = read(connfd, buff, sizeof(buff));
    size_t requestPtr = 0;

    Request request;

    request.method = SV_getRequestHttpMethod(buff, &requestPtr);
    request.path = SV_getRequestPath(buff, &requestPtr);
    request.content = SV_getRequestContent(buff, t);

    return request;
}

void SV_freeRequest(Request request) {
    free(request.path);
    free(request.content);
}

char* SV_solveRouteAndGetResponse(Server *s, Request request) {
    for (int i = 0; i < s->routesPtr; i++) {
        const Route currentRoute = s->routes[i];

        if (currentRoute.method == request.method && strcmp(currentRoute.path, request.path) == 0) {
            ResponseCreator* rc = currentRoute.callback(request);

            char* response = responseCreator_getResponse(rc);
            responseCreator_free(rc);

            return response;
        }
    }

    ResponseCreator* rc = responseCreator_init(TYPE_JSON, 404);

    char* response = responseCreator_getResponse(rc);
    responseCreator_free(rc);

    return response;
}

void SV_handleRequest(Server *s) {
    Request request = SV_parseRequest(s);

    char* response = SV_solveRouteAndGetResponse(s, request);

    write(s->connfd, response, strlen(response)-1);
    
    free(response);
    SV_freeRequest(request);
}

Server* server_init(uint16_t port) {
    Server* s = (Server*) malloc(sizeof(Server));

    if (s != NULL) {
        s->sockfd = 0;
        s->connfd = 0;
        
        s->port = port;

        s->routesPtr = 0;
    }

    return s;
}

void server_free(Server *s) {
    if (s->sockfd != 0)
        close(s->sockfd);

    if (s->connfd != 0)
        close(s->connfd);

    free(s);
}

void server_addRoute(Server *s, const char *path, 
                     enum http_method method, RouteCallback callback) {
    
    if (s->routesPtr == ROUTES_MAX_SIZE) {
        fprintf(stderr, "Server Error => server_addRoute: There is no more space to add routes\n");
        exit(1);
    }

    Route route = {
        .path = path,
        .method = method,
        .callback = callback,
    };

    s->routes[s->routesPtr] = route;
    s->routesPtr++;
}

void server_start(Server *s) {
    int cliLen;
    struct sockaddr_in servaddr, cli;
    struct hostent *hostp;

    s->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (s->sockfd == -1) {
        fprintf(stderr, "Server Error => server_start: Socket creation failed\n");
        exit(1);
    }

    int optval = 1;
    setsockopt(s->sockfd, SOL_SOCKET, SO_REUSEADDR, 
        (const void*) &optval, sizeof(int));

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(s->port);

    if ((bind(s->sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        fprintf(stderr, "Server Error => server_start: Socket bind failed\n");
        exit(1);
    }

    if (listen(s->sockfd, 5) != 0) {
        fprintf(stderr, "Server Error => server_start: Listen failed\n");
        exit(1);
    }

    printf("Server listening on port: %d\n", s->port);

    cliLen = sizeof(cli);

    while (true) {
        char *hostaddrp;

        s->connfd = accept(s->sockfd, (SA*)&cli, &cliLen);

        if (s->connfd < 0) {
            fprintf(stderr, "Server Error => server_start: Accept failed\n");
            exit(1);
        }

        hostp = gethostbyaddr((const char*) &cli.sin_addr.s_addr,
            sizeof(cli.sin_addr.s_addr), AF_INET);

        if (hostp == NULL) {
            fprintf(stderr, "Server Error => server_start: Error on gethostbyaddr\n");
            exit(1);
        }

        hostaddrp = inet_ntoa(cli.sin_addr);

        if (hostaddrp == NULL) {
            fprintf(stderr, "Server Error => server_start: Error on inet_ntoa\n");
            exit(1);
        }

        printf("Server established connection with %s (%s)\n", 
	        hostp->h_name, hostaddrp);

        SV_handleRequest(s);

        close(s->connfd);
    }

    close(s->sockfd);
}