#include <iostream>
#include <utility>

#include "lexer.h"
#include "parser.h"
#include "typechecker.h"
#include "cpp_backend.h"

int print_help(){
	std::cout<<"Program expected one argument, which is the file name.\n";
	return 1;
}

int main(int argc, char* argv[]){
	std::ostream& out = std::cout;
	if(argc != 2) {
		return print_help();
	}
	std::pair<std::pair<std::vector<typechecker::TypeError>, std::vector<typechecker::TypeError>>, std::vector<std::shared_ptr<const typechecker::RealClassInfo>>> typechecker_result;
	std::vector<ParsedModule> parsed_program;
	try {
		parsed_program = parse_program(argv[1]);
	} catch(const ParserError& e) {
		out << e;
		out << RED << "FAILURE!\n" << ENDCOLOUR;
		return 1;
	}
	typechecker_result = typechecker::typecheck(parsed_program);
	if(typechecker_result.first.first.empty()) {
		for(const ParsedModule& m : parsed_program) {
			print(m);
		}
	}
	typechecker::TypeError::print_errors(std::cout, parsed_program, typechecker_result.first);
	if(typechecker_result.first.first.empty()) {
		out << GREEN << "OK!\n" << ENDCOLOUR;
	} else {
		out << RED << "FAILURE!\n" << ENDCOLOUR;
		return 1;
	}
}
