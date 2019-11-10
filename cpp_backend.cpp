#include "typechecker.h"
#include "system.h"

using namespace ast;
using namespace expression;
using namespace statement;
using namespace typechecker;

namespace {

std::ostream& operator<<(std::ostream& o, const Identifier& i) {
	return o << i.name;
}

class OutputFile {
	std::ostream& o;
	size_t level;
	OutputFile() = delete;
public:
	OutputFile(std::ostream& o) : o(o), level(0) {}
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

	void print_type(const TypeInfo* type) {
		const RealClassInfo* as_real_class = dynamic_cast<const RealClassInfo*>(type);
		if(as_real_class) {
			RealClassInfoView view(*as_real_class);
			print_data("Type_", view.ast_data->name);
			if(!view.parameters.empty()) {
				print_data('<');
				print_type(view.parameters[0]);
				for(size_t i = 1; i < view.parameters.size(); ++i) {
					print_data(", ");
					print_type(view.parameters[i]);
				}
				print_data('>');
			}
			return;
		}
		if(dynamic_cast<const IntTypeInfo*>(type)) {
			print_data(u8"🍆::Integer");
			return;
		}
		if(dynamic_cast<const VoidTypeInfo*>(type)) {
			print_data("void");
			return;
		}
		throw std::runtime_error("Internal error.");
	}

	void print_type(const PointerType& t, const std::map<std::string, const TypeInfo*>& parameters) {
		if(parameters.find(t.name.name) != parameters.end()) {
			print_type(parameters.at(t.name.name));
			return;
		}
		o << u8"Type_" << t.name.name;
		if(!t.parameters.empty()) {
			o << '<';
			print_type(t.parameters[0], parameters);
			for(size_t i = 1; i < t.parameters.size(); ++i) {
				o << ", ";
				print_type(t.parameters[i], parameters);
			}
			o << '>';
		}
	}

	void print_type(const VariableType& t, const std::map<std::string, const TypeInfo*>& parameters) {
		return std::visit(overload(
		[&](const IntType&){
			o << u8"🍆::Integer";
		},
		[&](const PointerType& pt){
			print_type(pt, parameters);
		}
		), t);
	}

	void print_owned_type(const VariableType& type, const std::map<std::string, const TypeInfo*>& parameters) {
		print_data(u8"🍆::StrongObject<");
		print_type(type, parameters);
		print_data('>');
	}

	void print_owned_type(const TypeInfo* type) {
		if(dynamic_cast<const VoidTypeInfo*>(type)) {
			print_data("void");
			return;
		}
		print_data(u8"🍆::StrongObject<");
		print_type(type);
		print_data('>');
	}

	void print_function_arguments(const std::vector<std::pair<VariableType, Identifier>>& args, const std::map<std::string, const TypeInfo*>& parameters) {
		print_data("(");
		if(!args.empty()) {
			print_owned_type(args[0].first, parameters);
			print_data(" var_", args[0].second);
			for(size_t i = 1; i < args.size(); ++i) {
				print_data(", ");
				print_owned_type(args[i].first, parameters);
				print_data(" var_", args[i].second);
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
	void print_expression(const Expression& expression, const std::map<std::string, const TypeInfo*>& parameters) {
		static const char hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
		print_data("(");
		std::visit(overload(
		[&](const std::unique_ptr<StringLiteral>& e){
			print_data(u8"🍆::StrongObject<Type_String>::make_object((long long) \"");
			for(char c : e->value) {
				if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
					print_data(c);
				} else {
					print_data("\\x", hex_chars[(c & 0xF0) >> 4], hex_chars[c & 0x0F]);
				}
			}
			print_data("\", ", e->value.size(), "LL)");
		},
		[&](const std::unique_ptr<IntegerLiteral>& i){
			print_data(u8"🍆::StrongObject<🍆::Integer>(", i->value, "LL)");
		},
		[&](const std::unique_ptr<Identifier>& e){
			print_data("var_", *e);
		},
		[&](const std::unique_ptr<New>& e){
			print_data(u8"🍆::make_object<");
			print_type(e->type, parameters);
			print_data(">(");
			if(!e->args.empty()) {
				print_expression(e->args[0], parameters);
				for(size_t i = 1; i < e->args.size(); ++i) {
					print_data(", ");
					print_expression(e->args[i], parameters);
				}
			}
			print_data(")");
		},
		[&](const std::unique_ptr<Negation>& e){
			print_expression(e->value, parameters);
			print_data("->fun_", e->boolean ? NEG_FUN_NAME : OPP_FUN_NAME, "()");
		},
		[&](const std::unique_ptr<Cast>& e){
			print_data(u8"🍆::dcast<");
			print_type(e->target_type, parameters);
			print_data(">(");
			print_expression(e->value, parameters);
			print_data(")");
		},
		[&](const std::unique_ptr<Null>&){
			print_data(u8"🍆::null");
		},
		[&](const std::unique_ptr<This>&){
			print_data("get_this()");
		},
		[&](const std::unique_ptr<MemberAccess>& e){
			print_expression(e->object, parameters);
			print_data("->", "var_", e->member);
		},
		[&](const std::unique_ptr<FunctionCall>& e){
			if(e->lazy) {
				print_data(u8"🍆::lazy_call(");

				std::visit(overload([&](const std::unique_ptr<Identifier>&) {
					print_data("get_this()");
				},
				[&](const std::unique_ptr<MemberAccess>& ee) {
					print_expression(ee->object, parameters);
				},
				[&](const auto&) {
					throw std::runtime_error("Internal error.");
				}
				), e->function);

				print_data(", [](auto obj_ptr, auto... args) {return obj_ptr->");
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
				print_data("(args...);}");

				for(size_t i = 0; i < e->arguments.size(); ++i) {
					print_data(", ");
					print_expression(e->arguments[i].first, parameters);
				}

				print_data(")");
			} else {
				std::visit(overload([&](const std::unique_ptr<Identifier>& ee) {
					print_data("privfun_", *ee);
				},
				[&](const std::unique_ptr<MemberAccess>& ee) {
					print_expression(ee->object, parameters);
					print_data("->", "fun_", ee->member);
				},
				[&](const auto&) {
					throw std::runtime_error("Internal error.");
				}
				), e->function);
				print_data("(");
				if(!e->arguments.empty()) {
					print_expression(e->arguments[0].first, parameters);
					for(size_t i = 1; i < e->arguments.size(); ++i) {
						print_data(", ");
						print_expression(e->arguments[i].first, parameters);
					}
				}
				print_data(")");
			}
		},
		[&](const std::unique_ptr<BinaryOperation>& e){
			print_expression(e->left, parameters);
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
				print_expression(e->right, parameters);
				print_data(")");
			} else {
				if(e->type == BinaryOperationType::EQUAL) {
					print_data(" == ");
				} else {
					print_data(" != ");
				}
				print_expression(e->right, parameters);
			}
		}
		), expression);
		print_data(")");
	}

	void print_statement(const Statement& statement, bool print_semicolon, const std::map<std::string, const TypeInfo*>& parameters) {
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
			print_block(*s, parameters);
		},
		[&](const std::unique_ptr<Empty>&) {
			if(print_semicolon) {
				print_data(";");
			}
		},
		[&](const std::unique_ptr<StatementExpression>& s) {
			print_expression(s->expression, parameters);
			if(print_semicolon) {
				print_data(";");
			}
		},
		[&](const std::unique_ptr<For>& s) {
			print_data("for(");
			print_statement(s->before, true, parameters);
			print_data(" ");
			print_expression(s->condition, parameters);
			print_data("; ");
			print_statement(s->after, false, parameters);
			print_data(")");
			print_block(*s->body, parameters);
		},
		[&](const std::unique_ptr<VariableDeclaration>& s) {
			print_owned_type(s->type, parameters);
			print_data(" var_", s->name);
			if(s->value) {
				print_data(" = ");
				print_expression(*s->value, parameters);
			}
			if(print_semicolon) {
				print_data(";");
			}
		},
		[&](const std::unique_ptr<VariableAuto>& s) {
			print_data("auto var_", s->name);
			print_data(" = ");
			print_expression(s->value, parameters);
			if(print_semicolon) {
				print_data(";");
			}
		},
		[&](const std::unique_ptr<Return>& s) {
			print_data("return");
			if(!s->values.empty()) {
				print_data(" std::make_tuple(");
				print_expression(s->values[0], parameters);
				for(size_t i = 1; i < s->values.size(); ++i) {
					print_data(", ");
					print_expression(s->values[i], parameters);
				}
				print_data(")");
			}
			if(print_semicolon) {
				print_data(";");
			}
		},
		[&](const std::unique_ptr<If>& s) {
			print_data("if(");
			print_expression(s->condition, parameters);
			print_data(")");
			print_block(*s->body, parameters);
			if(s->body_else) {
				print_indentation();
				print_data("else ");
				print_block(**s->body_else, parameters);
			}
		},
		[&](const std::unique_ptr<While>& s) {
			print_data("while(");
			print_expression(s->condition, parameters);
			print_data(")");
			print_block(*s->body, parameters);
		},
		[&](const std::unique_ptr<Assignment>& s) {
			print_expression(s->variable, parameters);
			print_data(" = ");
			print_expression(s->value, parameters);
			if(print_semicolon) {
				print_data(";");
			}
		}
		), statement);
	}

	void print_block(const Block& b, const std::map<std::string, const TypeInfo*>& parameters) {
		print_data("{");
		print_endline();
		add_level();
		for(const Statement& s : b.statements) {
			print_indentation();
			print_statement(s, true, parameters);
			print_endline();
		}
		remove_level();
		print_line("}");
	}
};

}

namespace typechecker {

void print(std::ostream& o, const std::vector<const RealClassInfo*>& classes) {
	OutputFile output(o);

	output.print_line(u8"#include <🍆>");
	output.print_line();

	{
		std::set<std::string> patterns;
		for(const RealClassInfo* c_ : classes) {
			const Class& ast_data = *RealClassInfoView(*c_).ast_data;
			if(!patterns.count(ast_data.name.name)) {
				patterns.insert(ast_data.name.name);
				if(!ast_data.parameters.empty()) {
					output.print_indentation();
					output.print_data("template <typename");
					for(size_t i = 1; i < ast_data.parameters.size(); ++i) {
						output.print_data(", typename");
					}
					output.print_data('>');
					output.print_endline();
				}
				output.print_line(ast_data.public_variables ? "struct " : "class ", "Type_", ast_data.name, ';');
				output.print_line();
			}
		}
	}

	std::set<std::string> defined_templates;

	for(const RealClassInfo* c_ : classes) {
		const RealClassInfoView c(*c_);
		const Class& ast_data = *c.ast_data;

		std::map<std::string, const TypeInfo*> parameters;
		for(size_t i = 0; i < c.parameters.size(); ++i) {
			parameters.emplace(ast_data.parameters[i].name, c.parameters[i]);
		}

		{
			output.print_line("template <>");
			output.print_indentation();
			output.print_data("struct 🍆::has_journal<");
			output.print_type(c_);
			output.print_data("> : public std::", ast_data.nojournal ? "false" : "true", "_type {};");
			output.print_endline();
			output.print_line();
		}

		output.print_line(u8"#ifdef 🍆_Default_", ast_data.name);
		if(defined_templates.find(ast_data.name.name) == defined_templates.end()) {
			output.print_line(u8"🍆_Default_", ast_data.name);
		} else {
			defined_templates.insert(ast_data.name.name);
		}
		output.print_line(u8"#else");

		if(!c.parameters.empty()) {
			output.print_line("template <>");
		}

		output.print_indentation();
		output.print_data(ast_data.public_variables ? "struct " : "class ");

		output.print_type(c_);

		if(ast_data.superclass) {
			output.print_data(" : public ");
			output.print_type(*ast_data.superclass, parameters);
		}
		output.print_data(" {");
		output.print_endline();
		output.add_level();
		for(const ClassVariable& var : ast_data.variables) {
			std::visit(overload(
			[&](const ClassVariableInteger& i) {
				output.print_line(u8"🍆::StrongObject<🍆::Integer> var_", i.name, ';');
			},
			[&](const ClassVariablePointer& p) {
				output.print_indentation();
				output.print_data(u8"🍆::", p.strong ? "Strong" : "Weak", "Object<");
				output.print_type(p.type, parameters);
				output.print_data("> var_", p.name, ';');
				output.print_endline();
			}
			), var);
		}
		if(!ast_data.public_variables) {
			output.remove_level();
			output.print_line("public:");
			output.add_level();
		}
		if(!ast_data.nojournal && !ast_data.superclass) {
			output.print_line(u8"🍆::Journal journal;");
		}
		if(!ast_data.superclass) {
			output.print_line(u8"🍆::WeakObject<Type_" , ast_data.name, "> this_ptr;");
		}

		output.print_indentation();
		output.print_owned_type(c_);
		output.print_data(" get_this() {");
		output.print_endline();
		output.add_level();
		output.print_indentation();
		output.print_data(u8"return 🍆::scast<");
		output.print_type(c_);
		output.print_data(">(this_ptr);");
		output.print_endline();
		output.remove_level();
		output.print_line("}");

		{
			if(ast_data.destructor) {
				output.print_indentation();
				output.print_data("virtual ~Type_", ast_data.name, "() ");
				output.print_block(*ast_data.destructor->body, parameters);
				output.print_endline();
			} else {
				output.print_line("virtual ~Type_", ast_data.name, "() = default;");
			}
			output.print_line();
		}

		{
			bool has_default_constructor = false;
			for(const Constructor& constructor : ast_data.constructors) {
				if(constructor.arguments.empty()) {
					has_default_constructor = true;
				}
				output.print_indentation();
				output.print_data("Type_", ast_data.name);


				output.print_data("(WeakObject<");
				output.print_type(c_);
				output.print_data("> this_ptr");
				for(size_t i = 0; i < constructor.arguments.size(); ++i) {
					output.print_data(", ");
					output.print_owned_type(constructor.arguments[i].first, parameters);
					output.print_data(" var_", constructor.arguments[i].second);
				}
				output.print_data(")");

				output.print_data(' ');
				if(constructor.superclass_call) {
					output.print_data(": ");
					output.print_type(*ast_data.superclass, parameters);
					output.print_data("(this_ptr");
					if(!constructor.superclass_call->empty()) {
						for(size_t i = 0; i < constructor.superclass_call->size(); ++i) {
							output.print_data(", ");
							output.print_expression((*constructor.superclass_call)[i], parameters);
						}
					}
					output.print_data(") ");
				} else {
					output.print_data(": this_ptr(this_ptr) ");
				}
				output.print_block(*constructor.body, parameters);
				output.print_line();
			}
			if(!has_default_constructor) {
				output.print_line("Type_", ast_data.name, "() = delete;");
				output.print_line();
			}
			output.print_line("Type_", ast_data.name, "(const Type_", ast_data.name,"&) = delete;");
			output.print_line();
			output.print_line("Type_", ast_data.name, "(Type_", ast_data.name,"&&) = delete;");
			output.print_line();
		}

		{
			for(const auto& [name, f_] : c.functions) {
				RealFunctionInfoView f(*f_);

				if (f.body_ast && (*f.body_ast) == f.declaration_ast) {
					output.print_indentation();

					{
						bool void_return_type = dynamic_cast<const VoidTypeInfo*>(f.return_type);
						if(void_return_type && (!f.dual || f.dual->second.empty())) {
							output.print_data("void");
						} else {
							output.print_data("std::tuple<");
							if(!void_return_type) {
								output.print_owned_type(f.return_type);
							}
							if(f.dual && !f.dual->second.empty()) {
								if(!void_return_type) {
									output.print_data(", ");
								}
								output.print_owned_type(f.dual->second[0]);
								for(size_t i = 1; i < f.dual->second.size(); ++i) {
									output.print_data(", ");
									output.print_owned_type(f.dual->second[i]);
								}
							}
							output.print_data('>');
						}
					}

					output.print_data(" basefun_", f.name);
					output.print_function_arguments(f.declaration_ast->arguments, parameters);
					output.print_data(' ');
					output.print_block(**(*f.body_ast)->body, parameters);
					output.print_endline();

					output.print_indentation();
					output.print_data("virtual ");
					output.print_owned_type(f.return_type);
					output.print_data(" privfun_", f.name);
					output.print_function_arguments(f.declaration_ast->arguments, parameters);
					output.print_data(" {");
					output.print_endline();
					output.add_level();
					output.print_indentation();
					if(!dynamic_cast<const VoidTypeInfo*>(f.return_type)) {
						output.print_data("return std::get<0>(");
					} else {
						output.print_data('(');
					}
					output.print_data("basefun_", f.name, '(');
					if(!f.arguments.empty()) {
						output.print_data("var_", f.declaration_ast->arguments[0].second);
						for(size_t i = 1; i < f.arguments.size(); ++i) {
							output.print_data(", var_", f.declaration_ast->arguments[i].second);
						}
					}
					output.print_data("));");
					output.print_endline();
					output.remove_level();
					output.print_line('}');

					if(f.dual) {
						output.print_indentation();
						output.print_data("void dualfun_", f.name);
						if(f.declaration_ast->dual) {
							output.print_function_arguments(f.dual->first->first, parameters);
							output.print_data(' ');
							output.print_block(*f.dual->first->second, parameters);
						} else {
							auto superclass = std::make_unique<RealClassInfoView>(dynamic_cast<const RealClassInfo&>(**c.superclass));
							while(true) {
								RealFunctionInfoView df(*superclass->functions.at(f.name));
								if(df.declaration_ast->dual) {
									const Class& superclass_ast_data = *superclass->ast_data;
									std::map<std::string, const TypeInfo*> dual_parameters;
									for(size_t i = 0; i < superclass->parameters.size(); ++i) {
										dual_parameters.emplace(superclass_ast_data.parameters[i].name, superclass->parameters[i]);
									}
									output.print_function_arguments(df.dual->first->first, dual_parameters);
									output.print_data(' ');
									output.print_block(*df.dual->first->second, dual_parameters);
									break;
								}
								superclass = std::make_unique<RealClassInfoView>(dynamic_cast<const RealClassInfo&>(**superclass->superclass));
							}
						}
					}

					output.print_indentation();
					output.print_data("virtual ");
					output.print_owned_type(f.return_type);
					output.print_data(" fun_", f.name);
					output.print_function_arguments(f.declaration_ast->arguments, parameters);
					output.print_data(" {");
					output.print_endline();
					output.add_level();

					bool dual = false;
					if(f.declaration_ast->kind) {
						switch(*f.declaration_ast->kind) {
						case FunctionKind::DUAL:
							dual = true;
							break;
						case FunctionKind::NOEFFECT:
							dual = false;
							break;
						case FunctionKind::AUTO:
							dual = false;
							break;
						default:
							throw std::runtime_error("Internal error.");
						}
					}

					output.print_indentation();
					if(ast_data.nojournal) {
						if(!dynamic_cast<const VoidTypeInfo*>(f.return_type)) {
							output.print_data("return std::get<0>(");
						} else {
							output.print_data('(');
						}
						output.print_data("basefun_", f.name, '(');
						if(!f.arguments.empty()) {
							output.print_data("var_", f.declaration_ast->arguments[0].second);
							for(size_t i = 1; i < f.arguments.size(); ++i) {
								output.print_data(", var_", f.declaration_ast->arguments[i].second);
							}
						}
						output.print_data("));");
					} else {
						output.print_data("return journal.add_commit_", [&](){
							switch(*f.declaration_ast->kind) {
							case FunctionKind::DUAL:
								return "dual";
							case FunctionKind::NOEFFECT:
								return "noeffect";
							case FunctionKind::AUTO:
								return "auto";
							default:
								throw std::runtime_error("Internal error.");
							}
						}(), "(");
						output.print_data("get_this(), &");
						output.print_type(c_);
						output.print_data("::basefun_", f.name);
						if(dual) {
							output.print_data(", &");
							output.print_type(c_);
							output.print_data("::dualfun_", f.name);
						}
						for(size_t i = 0; i < f.arguments.size(); ++i) {
							output.print_data(", var_", f.declaration_ast->arguments[i].second);
						}
						output.print_data(");");
					}
					output.print_endline();
					output.remove_level();
					output.print_line('}');
				} else if (!f.body_ast) {
					output.print_indentation();
					output.print_data("virtual ");
					output.print_owned_type(f.return_type);
					output.print_data(" privfun_", f.name);
					output.print_function_arguments(f.declaration_ast->arguments, parameters);
					output.print_data(" = 0;");
					output.print_endline();

					output.print_indentation();
					output.print_data("virtual ");
					output.print_owned_type(f.return_type);
					output.print_data(" fun_", f.name);
					output.print_function_arguments(f.declaration_ast->arguments, parameters);
					output.print_data(" = 0;");
					output.print_endline();
				}
			}
		}

		output.remove_level();
		output.print_line("};");
		output.print_line("#endif");
		output.print_line();
	}
}
}
