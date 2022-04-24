# compilers_sandbox
A space for me to implement some compillers things

---
## Content:
0. [Basics](#0-basics)
1. [Lexer](#1-lexer)
2. [Extras](#2-extras)
---
## 0. Basics: 
### Compiling:
To compile everything, just execute in the root folder:
```sh
$ make all
```
Is possible to only compile the main compiler with:
```sh
$ make main
```
And to only compile the compiler with the server:
```sh
$ make server
```

### Executing:
A file called `a.out` will be created with only the compiler to you execute it with:
```sh
$ ./a.out
```
And a file called `server.out` with the server to you execute it with:
```sh
$ ./server.out
```

---
## 1. Lexer
One implementation of a Lexer to generate token from C based code.
### Usage:
1. The Lexer needs a Symbols Table to work, create it with:
```c
#include "symbolsTable/symbolsTable.h"

// ...

SymbolsTable* st = symbolsTable_init();
```
2. Now we can create the Lexer, it need 3 things: path to source code, the size of buffer and the Symbols Table:
```c
#include "lexer/lexer.h"

// ...

Lexer* l = lexer_init("code_example.txt", 1024, st);
```

3. And now we use `lexer_hasNext(Lexer*)` to check if has a Token available and `lexer_getNextToken(Lexer*)` to get the Token. Follow the example to get all Tokens:
```c
while (lexer_hasNext(l)) {
    Token t = lexer_getNextToken(l);
}
```

4. Don't forget to free the Lexer and the Symbols Table at the end:
```c
lexer_free(l);

symbolsTable_free(st);
```

> To a complete example see the [main.c](https://github.com/erikborella/compilers_sandbox/blob/main/main.c) file

---
## 2. Extras
### 2.1 Homemade server:

#### 1. Disclaimer

I created a web server from scratch using only TCP sockets, you can see more about it implementation in `extras/server` including the references that was consulted about sockets itself and HTTP responses.

However it's not perfect yet. It works to delivery content in json with simples requests (probably there's some bugs yet), but to serve HTTP pages like in Chrome it doesn't really work well because Chrome and other browers sends a lot of extras request that make the server crash in some point.

Also it's not complete yet, there's a lot of thing missing like some body parser (you can only deal with raw payload yet) and other things.

#### 2. Usage:
1. First we need to init the server specifying the port it will run.
```c
#include "extras/server/server.h"

// ...
int main() {
    Server* s = server_init(8000);

    return 0;
}
```

2. Now we define our callback that the server will call. The callbacks always need to return a `ResponseCreator*` and receive a `Request` as argument:
```c
#include "extras/server/server.h"
#include "extras/server/responseCreator/responseCreator.h"

// ...

//Callback definition
ResponseCreator* helloWorld(Request r) {
    
}
```

In the `Request` we can find some useful information like the HTTP method (GET, POST), the path of the request (/api/helloworld) and the raw payload sent in `content`.

3. Now we define the response that will send back to the client using the `ResponseCreator`. You only need to use `responseCreator_init` and specifying the content type (JSON, HTML) and the status code (200, 404), and then creting you response apedding strings with `responseCreator_appendContent`. Follow one example:
```c
//Callback definition
ResponseCreator* helloWorld(Request r) {
    ResponseCreator* response = responseCreator_init(TYPE_JSON, 200);

    responseCreator_appendContent(response, "[");

    responseCreator_appendContent(response, "{\"msg\": \"Hello, World!\"},");
    responseCreator_appendContent(response, "{\"msg\": \"It works!!\"}");

    responseCreator_appendContent(response, "]");

    return response;
}
```

And it will returns:
```json
[
    {
        "msg": "Hello, World!"
    },
    {
        "msg": "It works!!"
    }
]
```

> You don't need to worry about free the `ResponseCreator*` created, the server do it automatically

4. Now it just add our route to the server. You use `server_addRoute` to do it and specify the path (/api/helloworld), the method (GET, POST) and the callback:
```c
int main() {
    Server* s = server_init(8000);

    server_addRoute(s, "/api/helloworld", HTTP_GET, helloWorld);

    return 0;
}
```

5. And finally we start the server:
```c
int main() {
    //...

    //Start the server
    server_start(s);

    //Free the server
    server_free(s);

    return 0;
}
```
> You problaly will want to handle SIGINT (ctrl-c) to actualy free the server (currently there's not other way to stop it), see the [serverRunner.c](https://github.com/erikborella/compilers_sandbox/blob/main/serverRunner.c) file to an example.

### 2.2 Client
This is a simple web page created with [Vue](https://vuejs.org/), [Vuetify](https://vuetifyjs.com/) and the [Monaco Editor](https://microsoft.github.io/monaco-editor/) to a better code visualization.

1. First we need to install all the dependencies, make sure you have [NodeJs](https://nodejs.dev/) and [NPM](https://www.npmjs.com/) installed. in the `/extras/client` folder execute:
```sh
$ npm install
```
2. Now we just need to run that with:
```sh
$ npm run serve
```
> You also need to run the Compiler Server, see the [Basics](#0-basics) section
3. Acess `localhost:8080/` in your browser.

![Captura de tela de 2022-04-24 14-32-00](https://user-images.githubusercontent.com/27148919/164988967-4c249ecd-9f88-48a6-921d-4ddccb76b6bb.png)

