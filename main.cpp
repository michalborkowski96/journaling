#include "lexer.h"
#include "parser.h"
#include "typechecker.h"
#include "util.h"
#include "system.h"

#include <iostream>
#include <utility>
#include <numeric>
#include <fstream>

constexpr const char* const MAIN_NO_ARGS = "\nint main(){\n\treturn 🍆::make_object<Type_Main>()->fun_main().get_value();\n}";
constexpr const char* const MAIN_STRINGS_ARGS = "\n#include <cstring>\nint main(int argc, char* argv[]){\n\tauto args = 🍆::make_object<Type_Array<Type_String>>();\n\tfor(int i = 0; i < argc; ++i){\n\t\targs->fun_push_back(🍆::make_object<Type_String>(StrongObject<Integer>((long long) argv[i]), StrongObject<Integer>(strlen(argv[i]))));\n\t}\n\treturn 🍆::make_object<Type_Main>()->fun_main(args).get_value();\n}";

struct OptionDeclaration {
	std::string name;
	std::string description;
	std::vector<std::string> args;
	OptionDeclaration() = delete;
	OptionDeclaration(std::string name, std::string description, std::vector<std::string> args) : name(std::move(name)), description(std::move(description)), args(std::move(args)) {}

	std::string get_usage() const {
		std::string result = "-" + name;
		for(const std::string& arg : args) {
			result += " <" + arg + ">";
		}
		return result;
	}
};

const std::array<OptionDeclaration, 3> PROGRAM_OPTIONS {
	OptionDeclaration{"noe", "don't print optional warnings", {}},
	OptionDeclaration{"o", "output to file", {"filename"}},
	OptionDeclaration{"c", "append a main function to the result", {}},
};

template<size_t N>
void print_help(const std::array<OptionDeclaration, N>& program_options) {
	std::cerr << "USAGE:\n"
	          << "🍆 <source_file> [OPTIONS...]\n"
	          << "Options:\n";
	std::vector<std::pair<std::string, std::string>> entries;
	for(const OptionDeclaration& opt_decl : program_options) {
		entries.emplace_back(opt_decl.get_usage(), opt_decl.description);
	}
	size_t longest_usage = std::accumulate(entries.begin(), entries.end(), (size_t) 0, [](size_t a, const auto& e){return a > e.first.size() ? a : e.first.size();});
	for(const auto&[usage, description] : entries) {
		std::cerr << usage;
		for(size_t i = usage.size(); i < longest_usage + 4; ++i) {
			std::cerr << ' ';
		}
		std::cerr << description << '\n';
	}
}

template<size_t N>
std::map<std::string, std::vector<std::string>> get_options(const std::array<OptionDeclaration, N>& program_options, int argc, char* argv[]){
	int pos = 0;
	std::map<std::string, std::vector<std::string>> result;
	while(pos != argc) {
		bool found = false;
		for(const OptionDeclaration& opt_decl : program_options) {
			if((argv[pos] + 1) == opt_decl.name) {
				found = true;
				++pos;
				if(argc - pos < (int) opt_decl.args.size()) {
					throw std::runtime_error("Too few arguments for last option -" + opt_decl.name);
				}
				std::vector<std::string> args;
				for(size_t i = 0; i < opt_decl.args.size(); ++i) {
					args.emplace_back(argv[pos++]);
				}
				result.emplace(opt_decl.name, std::move(args));
				break;
			}
		}
		if(!found) {
			throw std::runtime_error("Unrecognized option " + std::string(argv[pos]));
		}
	}
	return result;
}

int main(int argc, char* argv[]){
	if(argc < 2) {
		print_help(PROGRAM_OPTIONS);
		return -1;
	}
	std::map<std::string, std::vector<std::string>> options;
	try {
		options = get_options(PROGRAM_OPTIONS, argc - 2, argv + 2);
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << '\n';
		return -1;
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
		const std::vector<const typechecker::RealClassInfo*> classes = typechecker_result.second.get_ordered();
		const char* end_append = nullptr;
		auto c = options.find("c");
		if(c != options.end()) {
			bool found = false;
			const typechecker::RealClassInfo* arr_string_type = nullptr;
			for(const typechecker::RealClassInfo* ci : classes) {
				if(ci->full_name() == "Array<String>") {
					arr_string_type = ci;
					break;
				}
			}
			for(const typechecker::RealClassInfo* ci : classes) {
				if(ci->full_name() == "Main") {
					found = true;
					auto fun = ci->get_function("main");
					if(!fun) {
						std::cerr << "-c option specified, but the 'Main' class has no 'main' function.\n";
						return -1;
					}
					if(!ci->constructible_with({})) {
						std::cerr << "-c option specified, but the 'Main' class doesn't have zero-argument constructor.\n";
						return -1;
					}
					const typechecker::RealFunctionInfo* fi = *fun;
					if(arr_string_type && fi->call_with({arr_string_type})) {
						const typechecker::TypeInfo* result = *fi->call_with({arr_string_type});
						if(!dynamic_cast<const typechecker::IntTypeInfo*>(result)) {
							std::cerr << "-c option specified, but the 'Main::main' class doesn't return 'int', but rather '" << result->full_name() << "'.\n";
							return -1;
						}
						end_append = MAIN_STRINGS_ARGS;
					} else if(fi->call_with({})) {
						const typechecker::TypeInfo* result = *fi->call_with({});
						if(!dynamic_cast<const typechecker::IntTypeInfo*>(result)) {
							std::cerr << "-c option specified, but the 'Main::main' class doesn't return 'int', but rather '" << result->full_name() << "'.\n";
							return -1;
						}
						end_append = MAIN_NO_ARGS;
					} else {
						std::cerr << "-c option specified, but the 'Main::main' function cannot be called with neither () arguments nor (Array<String>).\n";
						return -1;
					}
					break;
				}
			}
			if(!found) {
				std::cerr << "-c option specified, but no 'Main' class found.\n";
				return -1;
			}
		}
		auto o = options.find("o");
		std::ofstream output_file;
		std::ostream& out = o == options.end() ? std::cout : output_file;
		if(o != options.end()) {
			output_file.open(o->second[0]);
		}
		print(out, classes, get_executable_path().parent_path() / "stdlib" / "cpp" / u8"🍆");
		if(end_append) {
			out << end_append;
		}
		if(!out) {
			std::cerr << "Failed to write to file\n";
			return -1;
		}
	}
	typechecker::TypeError::print_errors(std::cerr, parsed_program, typechecker_result.first.first, RED);
	if(options.count("noe") == 0) {
		typechecker::TypeError::print_errors(std::cerr, parsed_program, typechecker_result.first.second, YELLOW);
	}
	if(typechecker_result.first.first.empty()) {
		std::cerr << GREEN << "OK!\n" << ENDCOLOUR;
	} else {
		std::cerr << RED << "FAILURE!\n" << ENDCOLOUR;
		return 1;
	}
}
