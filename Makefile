CC=gcc
CFLAGS=-O2  # to release compile
#CFLAGS=-O0 -g  # uncomment to debug

.PHONY: all main clean dist-clean

all: main clean

main: a.out
a.out: main.o lexer/lexer.o symbolsTable/symbolsTable.o lexer/bufferReader/bufferReader.o
	$(CC) $(CFLAGS) -o $@ $+

clean:
	find . -type f -name *.o -delete

dist-clean: clean
	rm -rf a.out
