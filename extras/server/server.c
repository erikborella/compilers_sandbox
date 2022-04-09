#include <stdio.h>
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

#define MAX 1000000
#define PORT 8080
#define SA struct sockaddr

/*
    TCP sockets inspired by:
        - https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
        - https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpserver.c

    HTTP responses inspired by:
        - https://www.tutorialspoint.com/http/http_responses.htm
*/

enum http_method {
    HTTP_GET,
    HTTP_POST,
    HTTP_OTHER,
};

typedef struct {
    enum http_method method;
    char *route;
    char *content;
} Request;

enum http_method getRequestHttpMethod(char request[MAX], size_t *requestPtr) {
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

char* getRequestRoute(char request[MAX], size_t *requestPtr) {
    size_t routeLen = 0;

    while (request[*requestPtr + routeLen] != ' ')
        routeLen++;

    const size_t mallocSize = sizeof(char) * routeLen + 1;
    char *route = malloc(mallocSize);
    bzero(route, mallocSize);

    strncpy(route, request + *requestPtr, routeLen);

    return route;
}

char* getRequestContent(char request[MAX], size_t requestLen) {
    char *contentStart = strstr(request, "\r\n\r\n");
    contentStart = contentStart + 4;

    const size_t contentLen = requestLen - (contentStart - request);
    const size_t mallocSize = sizeof(char) * contentLen;

    char *content = malloc(sizeof(char) * contentLen);
    bzero(content, mallocSize);

    strcpy(content, contentStart);

    return content;
}

Request parseRequest(int connfd) {
    char buff[MAX];
    bzero(buff, MAX);

    ssize_t t = read(connfd, buff, sizeof(buff));
    size_t requestPtr = 0;

    Request request;

    request.method = getRequestHttpMethod(buff, &requestPtr);
    request.route = getRequestRoute(buff, &requestPtr);
    request.content = getRequestContent(buff, t);

    printf("Route: %s\n", request.route);
    printf("Method: %d\n", request.method);
    printf("Content:\n%s\n\n", request.content);

    return request;
}

void freeRequest(Request request) {
    free(request.route);
    free(request.content);
}

void handleRequest(int connfd) {
    const char str[] = 
        "HTTP/1.1 200 OK\r\n"
        "Server: Integrated Compiler Server\r\n"
        "Content-Type: text/html\r\n"
        "Connection: Closed\r\n"
        "\r\n"
        "<html><body><h1>Hello, World!</h1></body></html>\r\n";

    Request r = parseRequest(connfd);

    freeRequest(r);

    write(connfd, str, sizeof(str)-1);
}

int main() {
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
    struct hostent *hostp;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Server Error: Socket creation failed\n");
        exit(1);
    }

    int optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int));

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        fprintf(stderr, "Server Error: Socket bind failed\n");
        exit(1);
    }

    if (listen(sockfd, 5) != 0) {
        fprintf(stderr, "Server Error: Listen failed\n");
        exit(1);
    }

    printf("Server listening on port: %d\n", PORT);

    len = sizeof(cli);

    while (true) {
        char *hostaddrp;

        connfd = accept(sockfd, (SA*)&cli, &len);

        if (connfd < 0) {
            fprintf(stderr, "Server Error: Accept failed\n");
            exit(1);
        }

        hostp = gethostbyaddr((const char*) &cli.sin_addr.s_addr, 
            sizeof(cli.sin_addr.s_addr), AF_INET);

        if (hostp == NULL) {
            fprintf(stderr, "Server Error: Error on gethostbyaddr\n");
            exit(1);
        }

        hostaddrp = inet_ntoa(cli.sin_addr);

        if (hostaddrp == NULL) {
            fprintf(stderr, "Server Error: Error on inet_ntoa\n");
            exit(1);
        }

        printf("server established connection with %s (%s)\n", 
	        hostp->h_name, hostaddrp);

        handleRequest(connfd);

        close(connfd);
    }

    close(sockfd);

    return 0;
}