#include "cpp_backend.h"

#include <fstream>

using namespace ast;
using namespace expression;
using namespace statement;

#warning CppBackendError
using CppBackendError = std::runtime_error;

std::ostream& operator<<(std::ostream& o, const VariableType& t);

std::ostream& operator<<(std::ostream& o, const PointerType& t) {
	o << u8"type_" << t.name.name;
	if(!t.parameters.empty()) {
		o << "<void";
		for(const auto& p : t.parameters) {
			o << ", " << p;
		}
		o << '>';
	}
	return o;
}

std::ostream& operator<<(std::ostream& o, const VariableType& t) {
	return std::visit(overload(
	[&](const IntType&)->std::ostream&{
	o << u8"🍆::Integer";
	return o;
	},
	[&](const PointerType& pt)->std::ostream&{
	o << pt;
	return o;
	}
	), t);
}

std::ostream& operator<<(std::ostream& o, const Type& t) {
	return std::visit(overload(
	[&](const VoidType&)->std::ostream&{
	o << "void";
	return o;
	},
	[&](const VariableType& vt)->std::ostream&{
	o << vt;
	return o;
	}
	), t);
}

class OutputFile {
	std::ofstream o;
	size_t level;
	OutputFile() = delete;
public:
	OutputFile(const std::string& filename) : level(0) {
		o.exceptions(std::ofstream::failbit | std::ofstream::badbit);
		o.open(filename);
	}
	void add_level() {
		++level;
	}
	void remove_level() {
		--level;
	}
	void print_indentation() {
		for(size_t i = 0; i < level; ++i) {
			o << '\t';
		}
	}
	template<typename... Ts>
	void print_line(const Ts&... args) {
		print_indentation();
		print_data(args...);
		print_endline();
	}
	void print_endline() {
		o << '\n';
	}
	void print_data() {}
	template<typename T, typename... Ts>
	void print_data(const T& arg, const Ts&... args) {
		o << arg;
		print_data(args...);
	}
};

void print(const ParsedModule& m) {
	std::string ext(FILENAME_EXTENSION);
	std::string filename = m.path;
	filename.resize(filename.size() - ext.size());
	filename += ".h";
	OutputFile header(filename);
	filename.back() = 'c';
	filename += "pp";
	OutputFile source(filename);

	header.print_line("#pragma once");
	header.print_line();

	auto is_acceptable_path = [](const std::string& path) {
		static const std::vector<std::string> forbidden{"'", "\\", "//", "/*"};
		for(const std::string& f : forbidden) {
			if(path.find(f) != path.npos) {
				return false;
			}
		}
		return true;
	};
	header.print_line(u8"#include <🍆>");
	header.print_line();

	for(const std::string& import : m.module.imports) {
		std::string import_header = import;
		import_header.resize(import_header.size() - ext.size());
		import_header += ".h";
		if(!is_acceptable_path(import_header)) {
			throw CppBackendError("Unsupported characters in imported module path " + import_header);
		}
		header.print_line("#include \"", import_header, '"');
	}
	header.print_line();

	for(const Class& c : m.module.classes) {
		if(!c.parameters.empty()) {
			header.print_indentation();
			header.print_data("template <");
			header.print_data(u8"typename typebase_", c.parameters[0].name);
			for(size_t i = 1; i < c.parameters.size(); ++i) {
				header.print_data(u8", typename type_", c.parameters[i].name);
			}
			header.print_data('>');
			header.print_endline();
		}
		header.print_indentation();
		header.print_data(c.public_variables ? "struct " : "class ", u8"type_", c.name.name);
		if(c.superclass) {
			header.print_data(" : public ", *c.superclass);
		}
		header.print_data(" {");
		header.print_endline();
		header.add_level();
		for(const ClassVariable& var : c.variables) {
			std::visit(overload(
			[&](const ClassVariableInteger& i) {
				header.print_line(u8"🍆::Integer ", i.name.name, ';');
			},
			[&](const ClassVariablePointer& p) {
				header.print_line(u8"🍆::", p.strong ? "Strong" : "Weak", "Object<" , p.type, "> ", p.name.name, ';');
			}
			), var);
		}
		header.remove_level();
		header.print_endline();
		header.print_data("}");
		header.print_endline();
		header.print_line();
	}
}
