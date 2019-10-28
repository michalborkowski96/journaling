#include "cpp_backend.h"
#include "system.h"

#include <fstream>

using namespace ast;
using namespace expression;
using namespace statement;

#warning CppBackendError
using CppBackendError = std::runtime_error;

std::ostream& operator<<(std::ostream& o, const VariableType& t);

std::ostream& operator<<(std::ostream& o, const PointerType& t) {
	o << u8"Type_" << t.name.name;
	o << "<void";
	for(const auto& p : t.parameters) {
		o << ", " << p;
	}
	o << '>';
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

std::ostream& operator<<(std::ostream& o, const Identifier& i) {
	return o << i.name;
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
	void print_function_arguments(const std::vector<std::pair<VariableType, Identifier>>& args) {
		print_data("(");
		if(!args.empty()) {
			print_data(u8"🍆::StrongObject<", args[0].first, "> var_", args[0].second);
			for(size_t i = 1; i < args.size(); ++i) {
				print_data(", ");
				print_data(u8"🍆::StrongObject<", args[i].first, "> var_", args[i].second);
			}
		}
		print_data(")");
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
	void print_expression(const Expression& expression) {
		static const char hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
		print_data("(");
		std::visit(overload(
		[&](const std::unique_ptr<StringLiteral>& e){
			print_data(u8"🍆::make_string(\"");
			for(char c : e->value) {
				if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
					print_data(c);
				} else {
					print_data("\\x", hex_chars[(c & 0xF0) >> 4], hex_chars[c & 0x0F]);
				}
			}
			print_data('"');
		},
		[&](const std::unique_ptr<IntegerLiteral>& i){
			print_data(i->value, "LL");
		},
		[&](const std::unique_ptr<Identifier>& e){
			print_data("var_", *e);
		},
		[&](const std::unique_ptr<New>& e){
			print_data(u8"🍆::make_object<", e->type, ">(");
			if(!e->args.empty()) {
				print_expression(e->args[0]);
				for(size_t i = 1; i < e->args.size(); ++i) {
					print_data(", ");
					print_expression(e->args[i]);
				}
			}
			print_data(")");
		},
		[&](const std::unique_ptr<Negation>& e){
			print_expression(e->value);
			print_data("->fun_", e->boolean ? NEG_FUN_NAME : OPP_FUN_NAME, "()");
		},
		[&](const std::unique_ptr<Cast>& e){
			print_data(u8"🍆::cast<", e->target_type, ">(");
			print_expression(e->value);
			print_data(")");
		},
		[&](const std::unique_ptr<Null>&){
			print_data(u8"🍆::null");
		},
		[&](const std::unique_ptr<This>&){
			print_data("_this");
		},
		[&](const std::unique_ptr<MemberAccess>& e){
			print_expression(e->object);
			print_data("->", "var_", e->member);
		},
		[&](const std::unique_ptr<FunctionCall>& e){
			std::visit(overload([&](const std::unique_ptr<Identifier>& ee) {
				print_data("privfun_", *ee);
			},
			[&](const std::unique_ptr<MemberAccess>& ee) {
				print_expression(ee->object);
				print_data("->", "fun_", ee->member);
			},
			[&](const auto&) {
				throw std::runtime_error("Internal error.");
			}
			), e->function);
			print_data("(");
			if(!e->arguments.empty()) {
				print_expression(e->arguments[0]);
				for(size_t i = 1; i < e->arguments.size(); ++i) {
					print_data(", ");
					print_expression(e->arguments[i]);
				}
			}
			print_data(")");
		},
		[&](const std::unique_ptr<LazyFunctionCall>& e){
			print_data(u8"🍆::lazy_call([");

			std::visit(overload([&](const std::unique_ptr<Identifier>&) {
				print_data(u8"_this{🍆::capture(_this)}");
			},
			[&](const std::unique_ptr<MemberAccess>& ee) {
				print_data(u8"_this{🍆::capture");
				print_expression(ee->object);
				print_data("}");
			},
			[&](const auto&) {
				throw std::runtime_error("Internal error.");
			}
			), e->function);

			for(size_t i = 0; i < e->arguments.size(); ++i) {
				print_data(", _arg", i, u8"{🍆::capture");
				print_expression(e->arguments[i].first);
				print_data("}");
			}

			print_data("](){return _this->");
			std::visit(overload([&](const std::unique_ptr<Identifier>& ee) {
				print_data("privfun_", *ee);
			},
			[&](const std::unique_ptr<MemberAccess>& ee) {
				print_data("fun_", ee->member);
			},
			[&](const auto&) {
				throw std::runtime_error("Internal error.");
			}
			), e->function);

			print_data('(');
			if(!e->arguments.empty()) {
				print_data("_arg0");
				for(size_t i = 1; i < e->arguments.size(); ++i) {
					print_data(", _arg", i);
				}
			}
			print_data(");}");
		},
		[&](const std::unique_ptr<BinaryOperation>& e){
			print_expression(e->left);
			const char* function_name = nullptr;
			switch(e->type) {
			case BinaryOperationType::MUL:
				function_name = MUL_FUN_NAME;
				break;
			case BinaryOperationType::DIV:
				function_name = DIV_FUN_NAME;
				break;
			case BinaryOperationType::ADD:
				function_name = ADD_FUN_NAME;
				break;
			case BinaryOperationType::SUB:
				function_name = SUB_FUN_NAME;
				break;
			case BinaryOperationType::OR:
				function_name = OR_FUN_NAME;
				break;
			case BinaryOperationType::AND:
				function_name = AND_FUN_NAME;
				break;
			case BinaryOperationType::MOD:
				function_name = MOD_FUN_NAME;
				break;
			case BinaryOperationType::LESS:
				function_name = LESS_FUN_NAME;
				break;
			case BinaryOperationType::LESS_EQUAL:
				function_name = LESS_EQUAL_FUN_NAME;
				break;
			case BinaryOperationType::MORE:
				function_name = MORE_FUN_NAME;
				break;
			case BinaryOperationType::MORE_EQUAL:
				function_name = MORE_EQUAL_FUN_NAME;
				break;
			case BinaryOperationType::EQUAL:
				break;
			case BinaryOperationType::NOT_EQUAL:
				break;
			default:
				throw std::runtime_error("Internal error.");
			}
			if(function_name != nullptr) {
				print_data("->fun_", function_name, "(");
				print_expression(e->right);
				print_data(")");
			} else {
				if(e->type == BinaryOperationType::EQUAL) {
					print_data(" == ");
				} else {
					print_data(" != ");
				}
				print_expression(e->right);
			}
		}
		), expression);
		print_data(")");
	}

	void print_statement(const Statement& statement, bool print_semicolon) {
		std::visit(overload(
		[&](const std::unique_ptr<Break>&) {
			print_data("break");
			if(print_semicolon) {
				print_data(";");
			}
		},
		[&](const std::unique_ptr<Continue>&) {
			print_data("continue");
			if(print_semicolon) {
				print_data(";");
			}
		},
		[&](const std::unique_ptr<Block>& s) {
			print_block(*s);
		},
		[&](const std::unique_ptr<Empty>&) {
			if(print_semicolon) {
				print_data(";");
			}
		},
		[&](const std::unique_ptr<StatementExpression>& s) {
			print_expression(s->expression);
			if(print_semicolon) {
				print_data(";");
			}
		},
		[&](const std::unique_ptr<For>& s) {
			print_data("for(");
			print_statement(s->before, true);
			print_data(" ");
			print_expression(s->condition);
			print_data("; ");
			print_statement(s->after, false);
			print_data(")");
			print_block(*s->body);
		},
		[&](const std::unique_ptr<VariableDeclaration>& s) {
			print_data(u8"🍆::StrongObject<", s->type, "> var_", s->name);
			if(s->value) {
				print_data(" = ");
				print_expression(*s->value);
			}
			if(print_semicolon) {
				print_data(";");
			}
		},
		[&](const std::unique_ptr<VariableAuto>& s) {
			print_data("auto var_", s->name);
			print_data(" = ");
			print_expression(s->value);
			if(print_semicolon) {
				print_data(";");
			}
		},
		[&](const std::unique_ptr<Return>& s) {
			print_data("return");
			if(!s->values.empty()) {
				print_data(u8" 🍆::make_return_value(");
				print_expression(s->values[0]);
				for(size_t i = 1; i < s->values.size(); ++i) {
					print_data(", ");
					print_expression(s->values[i]);
				}
				print_data(")");
			}
			if(print_semicolon) {
				print_data(";");
			}
		},
		[&](const std::unique_ptr<If>& s) {
			print_data("if(");
			print_expression(s->condition);
			print_data(")");
			print_block(*s->body);
			if(s->body_else) {
				print_indentation();
				print_data("else ");
				print_block(**s->body_else);
			}
		},
		[&](const std::unique_ptr<While>& s) {
			print_data("while(");
			print_expression(s->condition);
			print_data(")");
			print_block(*s->body);
		},
		[&](const std::unique_ptr<Assignment>& s) {
			print_expression(s->variable);
			print_data(" = ");
			print_expression(s->value);
			if(print_semicolon) {
				print_data(";");
			}
		}
		), statement);
	}

	void print_block(const Block& b) {
		print_data("{");
		print_endline();
		add_level();
		for(const Statement& s : b.statements) {
			print_indentation();
			print_statement(s, true);
			print_endline();
		}
		remove_level();
		print_line("}");
	}
};

void print(const ParsedModule& m) {
	std::string ext(FILENAME_EXTENSION);
	std::string header_filename = m.path;
	header_filename.resize(header_filename.size() - ext.size());
	header_filename += ".h";
	OutputFile header(header_filename);
	std::string source_filename = header_filename;
	source_filename.back() = 'c';
	source_filename += "pp";
	OutputFile source(source_filename);

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

	{
		std::string imported_header = get_file_name_from_absolute_path(header_filename);

		if(!is_acceptable_path(imported_header)) {
			throw CppBackendError("Unsupported characters in imported module path " + imported_header);
		}
		source.print_line("#include \"", imported_header, "\"");
		source.print_line();

		header.print_line(u8"#include <🍆>");
		header.print_line();

	}

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
		auto print_base_template_declaration = [&](){
			header.print_indentation();
			header.print_data("template <typename Void");
			for(const auto& p : c.parameters) {
				header.print_data(", typename Type_", p);
			}
			header.print_data('>');
			header.print_endline();
		};
		print_base_template_declaration();
		header.print_line(c.public_variables ? "struct " : "class Type_", c.name, ";");
		header.print_line();
		print_base_template_declaration();
		header.print_indentation();
		header.print_data(c.public_variables ? "struct " : "class ", "Base_", c.name);
		if(c.superclass) {
			header.print_data(" : public ", *c.superclass);
		}
		header.print_data(" {");
		header.print_endline();
		header.add_level();
		header.print_line(u8"🍆::WeakObject<Type_" , c.name, "> _this;");
		for(const ClassVariable& var : c.variables) {
			std::visit(overload(
			[&](const ClassVariableInteger& i) {
				header.print_line(u8"🍆::Integer var_", i.name, ';');
			},
			[&](const ClassVariablePointer& p) {
				header.print_line(u8"🍆::", p.strong ? "Strong" : "Weak", "Object<" , p.type, "> var_", p.name, ';');
			}
			), var);
		}
		header.remove_level();
		header.print_line("public:");
		header.add_level();

		{
			if(c.destructor) {
				header.print_indentation();
				header.print_data("~", c.name, "() ");
				header.print_block(*c.destructor->body);
				header.print_line();
			}
		}

		{
			bool has_default_constructor = false;
			for(const Constructor& constructor : c.constructors) {
				if(constructor.arguments.empty()) {
					has_default_constructor = true;
				}
				header.print_indentation();
				header.print_data("Base_", c.name);
				header.print_function_arguments(constructor.arguments);

				header.print_data(' ');
				if(constructor.superclass_call) {
					header.print_data(": ", *c.superclass, "(");
					if(!constructor.superclass_call->empty()) {
						header.print_expression((*constructor.superclass_call)[0]);
						for(size_t i = 1; i < constructor.superclass_call->size(); ++i) {
							header.print_data(", ");
							header.print_expression((*constructor.superclass_call)[i]);
						}
					}
					header.print_data(") ");
				}
				header.print_block(*constructor.body);
				header.print_line();
			}
			if(!has_default_constructor) {
				header.print_line(c.name, "() = delete;");
				header.print_line();
			}
			header.print_line(c.name, "(const ", c.name,"&) = delete;");
			header.print_line();
			header.print_line(c.name, "(", c.name,"&&) = delete;");
			header.print_line();
		}

		{
			for(const Function& f : c.functions) {
				if(f.dual) {
					header.print_indentation();
					header.print_data("virtual void dualfun_", f.name);
					header.print_function_arguments(f.dual->first);
					header.print_data(' ');
					header.print_block(*f.dual->second);
				}

				if(f.body) {
					header.print_indentation();
					header.print_data("auto basefun_", f.name);
					header.print_function_arguments(f.arguments);
					header.print_data(' ');
					header.print_block(**f.body);
					header.print_endline();
				}

				header.print_indentation();
				header.print_data("virtual ");
				std::visit(overload([&](const VoidType&){
					header.print_data("void");
				}, [&](const VariableType& t) {
					header.print_data(u8"🍆::StrongObject<", t, '>');
				}), f.return_type);
				header.print_data(" privfun_", f.name);
				header.print_function_arguments(f.arguments);
				header.print_data(" {");
				header.print_endline();
				header.print_indentation();
				header.print_data(u8"return 🍆::get_return_value<", f.return_type, ">(basefun_", f.name, '(');
				if(!f.arguments.empty()) {
					header.print_data(f.arguments[0].second);
					for(size_t i = 1; i < f.arguments.size(); ++i) {
						header.print_data(", ", f.arguments[1].second);
					}
				}
				header.print_data("));");
				header.print_endline();
				header.print_line('}');

				header.print_indentation();
				header.print_data("virtual ");
				std::visit(overload([&](const VoidType&){
					header.print_data("void");
				}, [&](const VariableType& t) {
					header.print_data(u8"🍆::StrongObject<", t, '>');
				}), f.return_type);
				header.print_data(" fun_", f.name);
				header.print_function_arguments(f.arguments);
				header.print_data(" {");
				header.print_endline();
				header.print_indentation();
				header.print_data(u8"auto result = basefun_", f.name, '(');
				if(!f.arguments.empty()) {
					header.print_data(f.arguments[0].second);
					for(size_t i = 1; i < f.arguments.size(); ++i) {
						header.print_data(", ", f.arguments[1].second);
					}
				}
				header.print_data(");");
				header.print_endline();
				header.print_line(u8"return 🍆::get_return_value<", f.return_type, ">(result);");
				header.print_line('}');

				header.print_line();
			}
		}

		header.remove_level();
		header.print_line("}");
		header.print_line();

		auto print_template_args = [&](OutputFile& o){
			if(c.parameters.empty()) {
				return;
			}
			o.print_data("<void");
			for(size_t i = 0; i < c.parameters.size(); ++i) {
				o.print_data(", Type_", c.parameters[i]);
			}
			o.print_data(">");
		};

		{
			print_base_template_declaration();
			header.print_indentation();
			header.print_data("struct 🍆::has_journal<Type_", c.name);
			print_template_args(header);
			header.print_data("> : public 🍆::", c.nojournal ? "false" : "true", "_type {}");
			header.print_endline();
			header.print_line();
		}
	}
}
