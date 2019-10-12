#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include <functional>
#include <vector>
#include <optional>
#include <map>
#include <set>

#include "ast.h"
#include "util.h"

using namespace ast;
using namespace expression;
using namespace statement;

constexpr const char* ADD_FUN_NAME = "add";
constexpr const char* SUB_FUN_NAME = "sub";
constexpr const char* MUL_FUN_NAME = "mul";
constexpr const char* DIV_FUN_NAME = "div";
constexpr const char* OR_FUN_NAME = "or";
constexpr const char* AND_FUN_NAME = "and";
constexpr const char* NEG_FUN_NAME = "neg";
constexpr const char* MOD_FUN_NAME = "mod";
constexpr const char* LESS_FUN_NAME = "less";
constexpr const char* LESS_EQUAL_FUN_NAME = "le";
constexpr const char* MORE_FUN_NAME = "more";
constexpr const char* MORE_EQUAL_FUN_NAME = "me";

constexpr const char* INT_TYPE_NAME = "int";
constexpr const char* VOID_TYPE_NAME = "void";
constexpr const char* STRING_TYPE_NAME = "String";
constexpr const char* NULL_TYPE_NAME = "null";
constexpr const char* TEMPLATE_UNKNOWN_TYPE_NAME = "_TemplateUnknownParameter";

constexpr const char* THIS_IDENTIFIER = "this";

class TypeError {
#warning TypeError
public:
	TypeError() = delete;
	TypeError(size_t, size_t, std::string);
	TypeError(const AstNode&, std::string);
	TypeError(const Expression&, std::string);
	TypeError(const Statement&, std::string);
};

template<typename T, typename U>
void call_with_error_log(std::vector<TypeError>& errors, std::variant<T, std::vector<TypeError>>&& data, U action_success){
	std::visit(overload(action_success, [&](std::vector<TypeError>&& err) {
		for(TypeError& e : err) {
			errors.push_back(std::move(e));
		}
	}), std::move(data));
}

template<typename T, typename U>
void call_with_error_log(std::vector<TypeError>& errors, const std::variant<T, std::vector<TypeError>>& data, U action_success){
	std::visit(overload(action_success, [&](const std::vector<TypeError>& err) {
		for(const TypeError& e : err) {
			errors.push_back(e);
		}
	}), data);
}

class TypeInfo;

class ClassDatabase {
#warning ClassDatabase
public:
	class CurrentClass {
#warning CurrentClass
		friend class ClassDatabase;
		CurrentClass(TypeInfo*);
	public:
		CurrentClass() = delete;
		CurrentClass(const CurrentClass&) = delete;
		CurrentClass(CurrentClass&&);
		~CurrentClass();
	};
	ClassDatabase() = delete;
	ClassDatabase(const ClassDatabase&) = delete;
	ClassDatabase(ClassDatabase&&) = delete;
	TypeInfo* get_for_int() const;
	TypeInfo* get_for_void() const;
	TypeInfo* get_for_template_unknown() const;
	TypeInfo* get_for_null() const;
	std::variant<TypeInfo*, std::vector<TypeError>> get(const Identifier&);
	std::variant<TypeInfo*, std::vector<TypeError>> get(const Type&);
	std::variant<TypeInfo*, std::vector<TypeError>> get(const VariableType&);
	std::variant<TypeInfo*, std::vector<TypeError>> get(const PointerType&);
	[[nodiscard]] std::variant<CurrentClass, std::vector<TypeError>> insert_temporary_type(std::shared_ptr<TypeInfo>);
};

std::string types_to_string(const std::vector<TypeInfo*>& v);

class FunctionInfo {
	const std::string name;
	const TypeInfo* return_type;
	const std::vector<TypeInfo*> arguments;
	const std::optional<const Function*> declaration_ast;
	const std::optional<const Block*> body_ast;
	const std::optional<const std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<statement::Block>>*> dual_ast;
	FunctionInfo(std::string name, TypeInfo* return_type, std::vector<TypeInfo*> arguments, std::optional<const Function*> declaration_ast, std::optional<const Block*> body_ast, std::optional<const std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<statement::Block>>*> dual_ast) : name(move(name)), return_type(std::move(return_type)), arguments(move(arguments)), declaration_ast(declaration_ast), body_ast(body_ast), dual_ast(dual_ast) {}
public:
	FunctionInfo() = delete;
	FunctionInfo(const FunctionInfo&) = delete;
	FunctionInfo(FunctionInfo&&) = delete;

	static std::variant<std::unique_ptr<FunctionInfo>, std::vector<TypeError>> make_function_declaration(std::optional<const FunctionInfo*> overridden, ClassDatabase& class_database, const Function* ast);
	std::optional<FunctionKind> kind() const {
		if(!declaration_ast) {
			return std::nullopt;
		}
		return (**declaration_ast).kind;
	}
	bool fully_defined() const {
		if(!declaration_ast) {
			return true;
		}
		if(kind() && (*kind()) == FunctionKind::DUAL) {
			return body_ast && dual_ast;
		} else {
			return (bool) body_ast;
		}
	}
	std::string full_name() const;
};

class TypeInfo {
	std::string name;
	std::vector<TypeInfo*> parameters;
	std::optional<TypeInfo*> superclass;
	std::map<std::string, TypeInfo*> variables;
	std::map<std::string, std::unique_ptr<FunctionInfo>> functions;
	std::map<size_t, std::vector<TypeInfo*>> constructors;
	bool class_type;
	bool variable_type;
	std::optional<const Class*> ast_data;
	TypeInfo() = delete;
	TypeInfo(const TypeInfo&) = delete;
	TypeInfo(TypeInfo&&) = delete;
	TypeInfo(decltype(name) name, decltype(parameters) parameters, decltype(superclass) superclass, bool class_type, bool variable_type, decltype(ast_data) ast_data) : name(move(name)), parameters(move(parameters)), superclass(move(superclass)), class_type(class_type), variable_type(variable_type), ast_data(move(ast_data)) {}
public:
	static std::variant<std::unique_ptr<TypeInfo>, std::vector<TypeError>> make_class(ClassDatabase& class_database, const Class* ast) {
		std::vector<TypeError> errors;
		std::shared_ptr<TypeInfo> type_info;
		{
			std::optional<TypeInfo*> superclass;
			if(ast->superclass) {
				call_with_error_log(errors, class_database.get(*ast->superclass), [&](TypeInfo* type){
					if(!type->ast_data) {
						throw std::runtime_error("Internal error.");
					}
					if((**type->ast_data).public_variables) {
						errors.emplace_back(*(ast->superclass), "Cannot extend struct-like classes.");
					} else {
						superclass = type;
					}
				});
			}

			std::vector<TypeInfo*> parameters;

			for(const Identifier& t : ast->parameters) {
				call_with_error_log(errors, class_database.get(t), [&](TypeInfo* type) {
					parameters.push_back(type);
				});
			}

			if(!errors.empty()) {
				return errors;
			}

			std::shared_ptr<TypeInfo> type_info = std::shared_ptr<TypeInfo>(new TypeInfo(ast->name.name, move(parameters), superclass, true, true, ast));
		}

		auto temp = class_database.insert_temporary_type(type_info);

		call_with_error_log(errors, temp, [&](const auto&){});

		if(!errors.empty()) {
			return errors;
		}

		{
			auto add_variable = [&](const ClassVariable& node, std::string name, TypeInfo* type){
				if(type_info->variables.find(name) != type_info->variables.end()) {
					std::visit([&](const auto& n){errors.emplace_back(n, "Redeclaration of a class variable.");}, node);
				} else {
					type_info->variables.insert(std::make_pair(std::move(name), type));
				}
			};

			for(const ClassVariable& var : ast->variables) {
				std::visit(overload(
				[&](const ClassVariableInteger& v){add_variable(var, v.name.name, class_database.get_for_int());},
				[&](const ClassVariablePointer& v){
					call_with_error_log(errors, class_database.get(v.type), [&](const auto& t){
						add_variable(var, v.name.name, t);
					});
				}
				), var);
			}
		}

		auto verify_function_declaration = [&](bool optional, const Function& f, const FunctionInfo& fi)->std::optional<std::vector<TypeError>>{
			std::vector<TypeError> errors;
			if(type_info->variables.find(f.name.name) != type_info->variables.end()) {
				errors.emplace_back(f.name, "Function shadows variable.");
			}
			if(type_info->functions.find(f.name.name) != type_info->functions.end()) {
				errors.emplace_back(f.name, "Function overload not supported.");
			}
			if(!ast->abstract && !fi.fully_defined()) {
				errors.emplace_back(f.name, "Function not fully defined in non-abstract class.");
			}
			if(ast->nojournal && !fi.kind()) {
				errors.emplace_back(f.name, "Nojournal class must not specify function kind.");
			}
			if(!ast->nojournal && fi.kind()) {
				errors.emplace_back(f.name, "Class with journal must specify function kind.");
			}
			if(optional && !fi.fully_defined()) {
				errors.emplace_back(f.name, "Optional functions must be fully defined.");
			}
			if(errors.empty()) {
				return std::nullopt;
			}
			return errors;
		};

		{
			auto check_argument_declaration = [&](const auto& f) -> std::vector<TypeInfo*> {
				std::vector<TypeInfo*> args;
				for(const auto&[type, ident] : f.arguments) {
					call_with_error_log(errors, class_database.get(type), [&](const auto& t){
						args.push_back(t);
					});
				}
				return args;
			};

			for(const Constructor& c : ast->constructors) {
				std::vector<TypeInfo*> args = check_argument_declaration(c);
				if(args.size() == c.arguments.size()) {
					if(type_info->constructors.find(args.size()) != type_info->constructors.end()) {
						errors.emplace_back(c.begin, c.body->begin, "Constructor overload only possible with a different number of arguments.");
					} else {
						type_info->constructors.insert(std::make_pair(args.size(), move(args)));
					}
				}
			}

			for(const Function& f : ast->functions) {
				std::optional<const FunctionInfo*> overridden;
				if(type_info->superclass) {
					overridden = (**type_info->superclass).get_function(f.name.name);
				}
				call_with_error_log(errors, FunctionInfo::make_function_declaration(overridden, class_database, &f), [&](std::unique_ptr<FunctionInfo>&& fi){
					auto e = verify_function_declaration(false, f, *fi);
					if(e) {
						for(TypeError& err : *e) {
							errors.emplace_back(std::move(err));
						}
					} else {
						type_info->functions.insert(std::make_pair(f.name.name, std::move(fi)));
					}
				});
			}
		}

		verify_function_body;

		verify_constructor_body_and_superclass_call;

		add_optional_functions;
	}
	std::optional<const FunctionInfo*> get_function(const std::string& name) {
		auto f = functions.find(name);
		if(f == functions.end()) {
			return std::nullopt;
		}
		return f->second.get();
	}
	static bool implicitly_convertible(const TypeInfo* t, const TypeInfo* o) {
		if(t->name == TEMPLATE_UNKNOWN_TYPE_NAME) {
			return true;
		}
		if(o->name == TEMPLATE_UNKNOWN_TYPE_NAME) {
			return true;
		}
		if(t->name == NULL_TYPE_NAME) {
			return true;
		}
		if(t == o) {
			return true;
		}
		const TypeInfo* superclass = t;
		while(superclass->superclass) {
			superclass = *superclass->superclass;
			if(superclass == o) {
				return true;
			}
		}
		return false;
	}
	std::string full_name() const {
		std::string ret = name;
		if(!parameters.empty()) {
			ret += "<";
			ret += parameters[0]->full_name();
			for(size_t i = 1; i < parameters.size(); ++i) {
				ret += ", ";
				ret += parameters[i]->full_name();;
			}
			ret += ">";
		}
		return ret;
	}
};

std::string types_to_string(const std::vector<TypeInfo*>& v) {
	std::string arg_types("(");
	if(v.size()) {
		arg_types = v[0]->full_name();
	}
	for(size_t i = 1; i < v.size(); ++i) {
		arg_types += ", ";
		arg_types += v[i]->full_name();
	}
	arg_types += ")";
	return arg_types;
}

std::variant<std::unique_ptr<FunctionInfo>, std::vector<TypeError>> FunctionInfo::make_function_declaration(std::optional<const FunctionInfo*> overridden, ClassDatabase& class_database, const Function* ast) {
	std::vector<TypeError> errors;
	TypeInfo* rt;
	std::vector<TypeInfo*> args;
	call_with_error_log(errors, class_database.get(ast->return_type), [&](TypeInfo* type){
		rt = type;
	});
	for(const auto& [type, ident] : ast->arguments) {
		call_with_error_log(errors, class_database.get(type), [&](TypeInfo* type){
			args.push_back(type);
		});
	}

	std::optional<const Block*> body;
	if(ast->body) {
		body = &**ast->body;
	} else if(overridden) {
		body = (**overridden).body_ast;
	}
	std::optional<const std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<statement::Block>>*> dual;
	if(ast->dual) {
		dual = &*ast->dual;
	} else if(overridden) {
		dual = (**overridden).dual_ast;
	}

	if(overridden) {
		const FunctionInfo& fi = **overridden;
		if(fi.arguments != args) {
			errors.emplace_back(ast->name, "Function overrides another function, but has different arguments, " + types_to_string(fi.arguments) + " instead of " + types_to_string(args) + ".");
		}
		if(!TypeInfo::implicitly_convertible(rt, fi.return_type)) {
			errors.emplace_back(ast->name, "Function overrides another function, but its return type " + rt->full_name() + " isn't implicitly convertible to " + fi.return_type->full_name() + ".");
		}
		if(ast->kind != fi.kind()) {
			errors.emplace_back(ast->name, "Function overrides another function, but has different kind.");
		}
	}

	if(!errors.empty()) {
		return errors;
	} else {
		return std::unique_ptr<FunctionInfo>(new FunctionInfo(ast->name.name, std::move(rt), move(args), ast, body, dual));
	}
}

std::string FunctionInfo::full_name() const {
	std::string n;
	n += return_type->full_name();
	n += " ";
	n += name;
	n += "(";
	if(arguments.size()) {
		n += arguments[0]->full_name();
		for(size_t i = 1; i < arguments.size(); ++i) {
			n += ", ";
			n += arguments[i]->full_name();
		}
	}
	n += ")";
	return n;
}

#endif
