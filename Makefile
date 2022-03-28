all: main clean

main: main.o lexer.o symbolsTable.o bufferReader.o
	gcc -o a.out main.o lexer.o symbolsTable.o bufferReader.o

main.o: main.c
	gcc -o main.o main.c -c

lexer.o: lexer/lexer.c
	gcc -o lexer.o lexer/lexer.c -c

symbolsTable.o: symbolsTable/symbolsTable.c
	gcc -o symbolsTable.o symbolsTable/symbolsTable.c -c

bufferReader.o: lexer/bufferReader/bufferReader.c
	gcc -o bufferReader.o lexer/bufferReader/bufferReader.c -c

clean:
	rm -rf *.o