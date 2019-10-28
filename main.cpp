#include <iostream>
#include <utility>

#include "lexer.h"
#include "parser.h"
#include "typechecker.h"

int print_help(){
	std::cerr<<"Program expected one argument, which is the file name.\n";
	return 1;
}

int main(int argc, char* argv[]){
	if(argc != 2) {
		return print_help();
	}
	std::pair<std::pair<std::vector<typechecker::TypeError>, std::vector<typechecker::TypeError>>, typechecker::ClassDatabase> typechecker_result;
	std::vector<ParsedModule> parsed_program;
	try {
		parsed_program = parse_program(argv[1]);
	} catch(const ParserError& e) {
		std::cerr << e;
		std::cerr << RED << "FAILURE!\n" << ENDCOLOUR;
		return 1;
	}
	typechecker_result = typechecker::typecheck(parsed_program);
	typechecker::TypeError::contract_errors(typechecker_result.first.first);
	typechecker::TypeError::contract_errors(typechecker_result.first.second);
	if(typechecker_result.first.first.empty()) {
		print(std::cout, typechecker_result.second.get_ordered());
	}
	typechecker::TypeError::print_errors(std::cerr, parsed_program, typechecker_result.first);
	if(typechecker_result.first.first.empty()) {
		std::cerr << GREEN << "OK!\n" << ENDCOLOUR;
	} else {
		std::cerr << RED << "FAILURE!\n" << ENDCOLOUR;
		return 1;
	}
}
