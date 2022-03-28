# compilers_sandbox
A space for me to implement some compillers things

---
## Content:
1. Lexer

---
## Compiling:
Just execute in the root folder:
```sh
$ make all
```
And a file called `a.out` will be created to you execute it with:
```sh
$ ./a.out
```

---
### 1. Lexer
One implementation of a Lexer to generate token from Java based code.
#### Usage:
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