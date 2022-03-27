#include "synbolsTable.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

struct symbol {
    size_t id;
    char* name; 
    struct symbol *next;
};

struct symbolsTable {
    struct symbol *head;
    int idCounter;
};

void* ST_mallocOrExitWithError(size_t size) {
    void* m = malloc(size);

    if (m == NULL) {
        fprintf(stderr, "Symbols Table Error: Unable to allocate %lu bytes\n", size);
    }

    return m;
}

size_t ST_findByName(SymbolsTable* st, char* name) {
    struct symbol *no = st->head;

    while (no != NULL) {
        if (strcmp(no->name, name) == 0)
            return no->id;
        else
            no = no->next;
    }

    return 0;
}

size_t ST_add(SymbolsTable* st, char* name) {
    struct symbol *no = ST_mallocOrExitWithError(sizeof(struct symbol));

    char *nameCopy = ST_mallocOrExitWithError(sizeof(char) * strlen(name));
    strcpy(nameCopy, name);

    st->idCounter++;
    no->id = st->idCounter;
    no->name = nameCopy;
    no->next = NULL;

    if (st->head == NULL) {
        st->head = no;
    }
    else {
        struct symbol *aux = st->head;

        while (aux->next != NULL)
            aux = aux->next;

        aux->next = no;
    }

    return no->id;
}

SymbolsTable* symbolsTable_init() {
    SymbolsTable* st = (SymbolsTable*) malloc(sizeof(SymbolsTable));

    if (st != NULL) {
        st->head = NULL;
        st->idCounter = 0;
    }

    return st;
}

void symbolsTable_free(SymbolsTable* st) {
    struct symbol *no;

    while (st->head != NULL) {
        no = st->head;
        st->head = st->head->next;

        free(no->name);
        free(no);
    }

    free(st);
}

size_t symbolsTable_getIdOrAddSymbol(SymbolsTable* st, char* symbolName) {
    size_t foundId = ST_findByName(st, symbolName);

    if (foundId != 0)
        return foundId;
    else
        return ST_add(st, symbolName);
}