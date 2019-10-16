FLAGS=-std=c++17 -O0 -g -Wall -Wextra
CC=g++-9

.PHONY: clean

all: aubergine

%.o: %.cpp *.h
	${CC} $< ${FLAGS} -c

aubergine: ast.o cpp_backend.o lexer.o main.o parser.o system_posix.o system.h typechecker.o util.o
	${CC} *.o -o aubergine

clean:
	rm *.o
	rm aubergine
