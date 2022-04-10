#include "extras/server/server.h"
#include "extras/server/responseCreator/responseCreator.h"

ResponseCreator* root(Request r) {
    ResponseCreator* rc = responseCreator_init(TYPE_JSON, 200);

    responseCreator_appendContent(rc, "{\"msg\": \"UHULLLLLL\"}");

    return rc;
}

int main() {

    Server* s = server_init(8080);

    server_addRoute(s, "/", HTTP_GET, root);

    server_start(s);

    server_free(s);

    return 0;
}