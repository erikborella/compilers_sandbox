CC=gcc

.PHONY: all main clean

all: main clean

main: a.out
a.out: main.o lexer/lexer.o symbolsTable/symbolsTable.o lexer/bufferReader/bufferReader.o
	$(CC) -o $@ $+

clean:
	find . -type f -name *.o -delete
