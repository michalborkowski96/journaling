#include <iostream>
#include <utility>

#include "lexer.h"
#include "parser.h"
#include "typechecker.h"

int print_help(){
	std::cout<<"Program expected one argument, which is the file name.\n";
	return 1;
}

int main(int argc, char* argv[]){
	std::ostream& out = std::cout;
	if(argc != 2) {
		return print_help();
	}
	std::pair<std::vector<TypeError>, std::vector<TypeError>> typechecker_errors;
	std::vector<ParsedModule> parsed_program;
	try {
		parsed_program = parse_program(argv[1]);
	} catch(const ParserError& e) {
		out << e;
	}
	typechecker_errors = typecheck(parsed_program);
	TypeError::print_errors(std::cout, parsed_program, typechecker_errors);
}
