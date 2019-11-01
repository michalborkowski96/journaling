FLAGS=-std=c++17 -O0 -g -Wall -Wextra
CC=clang++-8

.PHONY: clean

all: 🍆

%.o: %.cpp *.h
	${CC} $< ${FLAGS} -c

🍆: ast.o cpp_backend.o lexer.o main.o parser.o system_posix.o system_posix.o typechecker.o util.o
	${CC} $^ -o 🍆

clean:
	rm *.o
	rm 🍆
