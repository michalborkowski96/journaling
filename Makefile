FLAGS=-std=c++17 -O0 -g -Wall -Wextra
CC=g++-9

.PHONY: clean

all: aubergine

aubergine: ast.o cpp_backend.o lexer.o main.o parser.o system_posix.o system.h typechecker.o util.o

%.o: %.c %.h
	CC %.c FLAGS -c

clean:
	rm *.o
	rm aubergine
