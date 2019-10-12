#include <iostream>
#include <utility>

#include "lexer.h"
#include "parser.h"


int print_help(){
	std::cout<<"Program expected one argument, which is the file name.\n";
	return 1;
}

int main(int argc, char* argv[]){
	std::ostream& out = std::cout;
	if(argc != 2) {
		return print_help();
	}
	try {
		parse_program(argv[1]);
	} catch(const ParserError& e) {
		out << e;
	}
}
