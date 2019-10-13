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

constexpr const char* THIS_IDENTIFIER = "this";

struct TypeError {
#warning TypeError
	TypeError() = delete;
	TypeError(size_t, size_t, std::string);
	TypeError(const AstNode&, std::string);
	TypeError(const Expression&, std::string);
	TypeError(const Statement&, std::string);
};

class TypeRegister {
public:
	struct TypeInfo;

	struct FunctionInfo {
		std::string name;
		std::shared_ptr<TypeInfo> return_type;
		std::vector<std::shared_ptr<TypeInfo>> arguments;
#warning	AST
		std::optional<const Function*> ast;
		FunctionInfo() = delete;
		FunctionInfo(const FunctionInfo&) = delete;
		FunctionInfo(FunctionInfo&&) = delete;
		FunctionInfo(std::string name, std::shared_ptr<TypeInfo> return_type, std::vector<std::shared_ptr<TypeInfo>> arguments, std::optional<const Function*> ast) : name(move(name)), return_type(std::move(return_type)), arguments(move(arguments)), ast(ast) {}
		std::string full_name() const {
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
	};

	struct TypeInfo {
		std::string name;
		std::vector<std::shared_ptr<TypeInfo>> parameters;
		std::optional<std::shared_ptr<TypeInfo>> superclass;
		std::map<std::string, std::shared_ptr<TypeInfo>> variables;
		std::map<std::pair<std::string, size_t>, std::shared_ptr<FunctionInfo>> own_functions;
		std::map<std::pair<std::string, size_t>, std::shared_ptr<FunctionInfo>> inherited_functions;
		std::map<size_t, std::vector<std::shared_ptr<TypeInfo>>> constructors;
		bool class_type;
		bool variable_type;
		std::optional<const Class*> ast_data;
		TypeInfo() = delete;
		TypeInfo(const TypeInfo&) = delete;
		TypeInfo(TypeInfo&&) = delete;
		TypeInfo(std::string name, std::vector<std::shared_ptr<TypeInfo>> parameters, std::optional<std::shared_ptr<TypeInfo>> superclass, std::map<std::string, std::shared_ptr<TypeInfo>> variables, std::map<std::pair<std::string, size_t>, std::shared_ptr<FunctionInfo>> inherited_functions, std::map<size_t, std::vector<std::shared_ptr<TypeInfo>>> constructors, bool class_type, bool variable_type, std::optional<const Class*> ast_data) : name(move(name)), parameters(move(parameters)), superclass(move(superclass)), variables(move(variables)), inherited_functions(move(inherited_functions)), constructors(move(constructors)), class_type(class_type), variable_type(variable_type), ast_data(move(ast_data)) {}

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

		std::optional<TypeError> insert_function(std::string name, std::shared_ptr<TypeInfo> return_type, std::vector<std::shared_ptr<TypeInfo>> arguments, std::optional<const Function*> ast) {
#warning UHMMMMM
			auto p = std::make_pair(std::move(name), arguments.size());
			if(own_functions.find(p) != own_functions.end()) {
				if(own_functions.at(p)->ast) {
					return TypeError(**(own_functions.at(p)->ast), "Function overload possible only with a different number of arguments.");
				} else {
					throw std::runtime_error("Internal error.");
				}
			}
			{
				auto inh_fun = inherited_functions.find(p);
				if(inh_fun != inherited_functions.end()) {
					if(ast && inh_fun->second->ast) {
						auto& k = (**inh_fun->second->ast).kind;
						if(k != (**ast).kind) {
							return TypeError((**ast).name, "Function override must be with exactly same kinds.");
						}
					}

					for(size_t i = 0; i < arguments.size(); ++i) {
						if(arguments[i] != inh_fun->second->arguments[i]) {
							if(ast) {
								return TypeError((**ast).name, "Function override must be with exactly same types.");
							} else {
								throw std::runtime_error("Internal error.");
							}
						}
					}
				}
			}
			std::shared_ptr<FunctionInfo>& f = own_functions[p];
			f = std::make_shared<FunctionInfo>(std::move(p.first), return_type, std::move(arguments), ast);
			return std::nullopt;
		}

		std::map<std::pair<std::string, size_t>, std::shared_ptr<FunctionInfo>> get_functions() const {
			auto ret = inherited_functions;
			for(const auto&[key, function] : own_functions) {
				ret[key] = function;
			}
			return ret;
		}

		std::optional<std::vector<TypeError>> check_purely_virtual_functions() const {
			std::vector<TypeError> errors;
			if(ast_data && (!(**ast_data).abstract)) {
				auto functions = get_functions();
				for(const auto&[key, function] : functions) {
					if(!function->ast) {
						throw std::runtime_error("Internal error.");
					}
					if(!(**(function->ast)).body) {
						errors.emplace_back(**(function->ast), "Purely virtual function used by non-abstract class " + name + ".");
					}
				}
			}
			if(errors.empty()) {
				return std::nullopt;
			}
			return errors;
		};
	};

	TypeRegister() {
		std::shared_ptr<TypeInfo> int_info = std::make_shared<TypeInfo>(INT_TYPE_NAME, std::vector<std::shared_ptr<TypeInfo>>{}, std::optional<std::shared_ptr<TypeInfo>>{}, std::map<std::string, std::shared_ptr<TypeInfo>>{}, std::map<std::pair<std::string, size_t>, std::shared_ptr<FunctionInfo>>{}, std::map<size_t, std::vector<std::shared_ptr<TypeInfo>>>{}, false, true, std::optional<const Class*>{});

		int_info->insert_function(ADD_FUN_NAME, int_info, {int_info}, std::nullopt);
		int_info->insert_function(SUB_FUN_NAME, int_info, {int_info}, std::nullopt);
		int_info->insert_function(MUL_FUN_NAME, int_info, {int_info}, std::nullopt);
		int_info->insert_function(DIV_FUN_NAME, int_info, {int_info}, std::nullopt);
		int_info->insert_function(OR_FUN_NAME, int_info, {int_info}, std::nullopt);
		int_info->insert_function(AND_FUN_NAME, int_info, {int_info}, std::nullopt);
		int_info->insert_function(NEG_FUN_NAME, int_info, {}, std::nullopt);
		int_info->insert_function(MOD_FUN_NAME, int_info, {int_info}, std::nullopt);
		int_info->insert_function(LESS_FUN_NAME, int_info, {int_info}, std::nullopt);
		int_info->insert_function(LESS_EQUAL_FUN_NAME, int_info, {int_info}, std::nullopt);
		int_info->insert_function(MORE_FUN_NAME, int_info, {int_info}, std::nullopt);
		int_info->insert_function(MORE_EQUAL_FUN_NAME, int_info, {int_info}, std::nullopt);

		types.push_back(int_info);

		types.emplace_back(std::make_shared<TypeInfo>(VOID_TYPE_NAME, std::vector<std::shared_ptr<TypeInfo>>{}, std::optional<std::shared_ptr<TypeInfo>>{}, std::map<std::string, std::shared_ptr<TypeInfo>>{}, std::map<std::pair<std::string, size_t>, std::shared_ptr<FunctionInfo>>{}, std::map<size_t, std::vector<std::shared_ptr<TypeInfo>>>{}, false, false, std::optional<const Class*>{}));

		types.emplace_back(std::make_shared<TypeInfo>(NULL_TYPE_NAME, std::vector<std::shared_ptr<TypeInfo>>{}, std::optional<std::shared_ptr<TypeInfo>>{}, std::map<std::string, std::shared_ptr<TypeInfo>>{}, std::map<std::pair<std::string, size_t>, std::shared_ptr<FunctionInfo>>{}, std::map<size_t, std::vector<std::shared_ptr<TypeInfo>>>{}, false, false, std::optional<const Class*>{}));
	}

	size_t find(const std::string& name, const std::vector<std::shared_ptr<TypeInfo>>& parameters) const {
		size_t i;
		for(i = 0; i < types.size(); ++i) {
			if(types[i]->name != name) {
				continue;
			}
			if(types[i]->parameters != parameters) {
				continue;
			}
			break;
		}
		return i;
	}

	size_t size() const {
		return types.size();
	}

	std::shared_ptr<TypeInfo> get_for_void() const {
		return types[1];
	}

	std::shared_ptr<TypeInfo> get_for_int() const {
		return types[0];
	}

	std::shared_ptr<TypeInfo> get_for_null() const {
		return types[2];
	}

	const TypeInfo& operator[](size_t i) const {
		return *(types[i]);
	}

	std::shared_ptr<TypeInfo> at(size_t i) const {
		return types[i];
	}

	[[nodiscard]] bool add(std::shared_ptr<TypeInfo> ti){
		if(find(ti->name, ti->parameters) >= size()) {
			types.push_back(std::move(ti));
			return true;
		}
		return false;
	}
private:
	std::vector<std::shared_ptr<TypeInfo>> types;
};

class ExpressionCheckResult {
	std::variant<std::pair<std::shared_ptr<TypeRegister::TypeInfo>, bool>, std::vector<std::shared_ptr<TypeRegister::FunctionInfo>>> type;
	ExpressionCheckResult(std::vector<std::shared_ptr<TypeRegister::FunctionInfo>> funs) : type(std::move(funs)) {}
	static bool implicit_convertible(const TypeRegister::TypeInfo* t, const TypeRegister::TypeInfo* o) {
		if(t->name == NULL_TYPE_NAME) {
			return true;
		}
		if(t == o) {
			return true;
		}
		const TypeRegister::TypeInfo* superclass = t;
		while(superclass->superclass) {
			superclass = superclass->superclass->get();
			if(superclass == o) {
				return true;
			}
		}
		return false;
	}
public:
	ExpressionCheckResult() = delete;
	ExpressionCheckResult(std::shared_ptr<TypeRegister::TypeInfo> var, bool lvalue) : type(std::make_pair(var, lvalue)) {}
	bool implicit_convertible_to(const ExpressionCheckResult& o) const {
		return std::visit(overload(
		[&](const std::pair<std::shared_ptr<TypeRegister::TypeInfo>, bool>& d){
			return implicit_convertible_to(d.first.get());
		},
		[&](const std::vector<std::shared_ptr<TypeRegister::FunctionInfo>>&) {
			return false;
		}
		), o.type);
	}
	bool implicit_convertible_to(const TypeRegister::TypeInfo* o) const {
		return std::visit(overload(
		[&](const std::pair<std::shared_ptr<TypeRegister::TypeInfo>, bool>& d){
			return implicit_convertible(d.first.get(), o);
		},
		[&](const std::vector<std::shared_ptr<TypeRegister::FunctionInfo>>&) {
			return false;
		}
		), type);
	}
	bool explicit_convertible_to(const ExpressionCheckResult& o) const {
		return implicit_convertible_to(o) || o.implicit_convertible_to(*this);
	}
	bool explicit_convertible_to(const TypeRegister::TypeInfo* o) const {
		return std::visit(overload(
		[&](const std::pair<std::shared_ptr<TypeRegister::TypeInfo>, bool>& d){
			return implicit_convertible(d.first.get(), o) || implicit_convertible(o, d.first.get());
		},
		[&](const std::vector<std::shared_ptr<TypeRegister::FunctionInfo>>&) {
			return false;
		}
		), type);
	}
	std::optional<ExpressionCheckResult> call_with(const std::vector<ExpressionCheckResult>& args) const {
		return std::visit(overload(
		[&](const std::pair<std::shared_ptr<TypeRegister::TypeInfo>, bool>&)->std::optional<ExpressionCheckResult>{
			return std::nullopt;
		},
		[&](const std::vector<std::shared_ptr<TypeRegister::FunctionInfo>>& funs)->std::optional<ExpressionCheckResult>{
			for(const auto& f : funs) {
				if(f->arguments.size() != args.size()) {
					continue;
				}
				size_t i;
				for(i = 0; i < args.size(); ++i) {
					if(!args[i].implicit_convertible_to(f->arguments[i].get())) {
						break;
					}
				}
				if(i == args.size()) {
					return ExpressionCheckResult(f->return_type, false);
				}
			}
			return std::nullopt;
		}
		), type);
	}
	std::string full_name() const {
		return std::visit(overload(
		[&](const std::pair<std::shared_ptr<TypeRegister::TypeInfo>, bool>& d){
			return d.first->full_name();
		},
		[&](const std::vector<std::shared_ptr<TypeRegister::FunctionInfo>>& f) {
			if(f.empty()) {
				throw std::runtime_error("Internal error.");
			}
			std::string n = f[0]->full_name();
			for(size_t i = 0; i < f.size(); ++i) {
				n += " or ";
				n += f[i]->full_name();
			}
			return n;
		}
		), type);
	}
	bool is_lvalue() const {
		return std::visit(overload(
		[&](const std::pair<std::shared_ptr<TypeRegister::TypeInfo>, bool>& d){
			return d.second;
		},
		[&](const std::vector<std::shared_ptr<TypeRegister::FunctionInfo>>&) {
			return false;
		}
		), type);
	}
	bool check_lazy_call_journal_mark(bool marked_nojournal) const {
		return std::visit(overload(
		[&](const std::pair<std::shared_ptr<TypeRegister::TypeInfo>, bool>& d){
			if(d.first->ast_data) {
				return (**d.first->ast_data).nojournal == marked_nojournal;
			} else {
				return !marked_nojournal;
			}
		},
		[&](const std::vector<std::shared_ptr<TypeRegister::FunctionInfo>>&) {
			return !marked_nojournal;
		}
		), type);
	}
	bool comparable_with(const ExpressionCheckResult& o) const {
		return explicit_convertible_to(o);
	}
	std::optional<ExpressionCheckResult> get_member(const std::string& name) const {
		return std::visit(overload(
		[&](const std::pair<std::shared_ptr<TypeRegister::TypeInfo>, bool>& d)->std::optional<ExpressionCheckResult>{
			auto v = d.first->variables.find(name);
			if(v != d.first->variables.end()) {
				return ExpressionCheckResult(v->second, true);
			}
			std::vector<std::shared_ptr<TypeRegister::FunctionInfo>> funs;
			for(const auto&[meta, data] : d.first->get_functions()) {
				if(meta.first == name) {
					funs.push_back(data);
				}
			}
			if(funs.empty()) {
				return std::nullopt;
			}
			return ExpressionCheckResult(std::move(funs));
		},
		[&](const std::vector<std::shared_ptr<TypeRegister::FunctionInfo>>&)->std::optional<ExpressionCheckResult>{
			return std::nullopt;
		}
		), type);
	}
};

class PatternRegister {
	std::map<std::string, const Class*> patterns;
public:
	const Class* operator[](const std::string& str) const {
		return patterns.at(str);
	}

	bool has(const std::string& str) const {
		return patterns.find(str) != patterns.end();
	}

	void add(std::string str, const Class* c) {
		patterns.insert(std::make_pair(move(str), c));
	}
};

class Typechecker {
	PatternRegister patterns;
	TypeRegister types;

	template<typename T, typename U>
	void instantiate_type(const std::string& outer_class, const std::map<std::string, std::shared_ptr<TypeRegister::TypeInfo>>& environmental_parameters, const T& type, std::vector<TypeError>& errors, U action) {
		std::visit(overload(
		[&](std::vector<TypeError>&& e){
			for(TypeError& ee : e) {
				errors.emplace_back(std::move(ee));
			}
		},
		[&](std::optional<std::shared_ptr<TypeRegister::TypeInfo>>&& iter){
			action(iter);
		}
		), instantiate_type(outer_class, environmental_parameters, type));
	}

	std::variant<std::vector<TypeError>, std::optional<std::shared_ptr<TypeRegister::TypeInfo>>> instantiate_type(const std::string& outer_class, const std::map<std::string, std::shared_ptr<TypeRegister::TypeInfo>>& environmental_parameters, const Type& type) {
		return std::visit(overload(
		[&](const VariableType& t){
			return instantiate_type(outer_class, environmental_parameters, t);
		},
		[&](const VoidType&)->std::variant<std::vector<TypeError>, std::optional<std::shared_ptr<TypeRegister::TypeInfo>>>{
			return types.get_for_void();
		}
		), type);
	}

	std::variant<std::vector<TypeError>, std::optional<std::shared_ptr<TypeRegister::TypeInfo>>> instantiate_type(const std::optional<std::string>& outer_class, const std::map<std::string, std::shared_ptr<TypeRegister::TypeInfo>>& environmental_parameters, const PointerType& type) {
		return instantiate_type(outer_class, environmental_parameters, type, type.name.name, type.parameters);
	}

	std::variant<std::vector<TypeError>, std::optional<std::shared_ptr<TypeRegister::TypeInfo>>> instantiate_type(const std::optional<std::string>& outer_class, const std::map<std::string, std::shared_ptr<TypeRegister::TypeInfo>>& environmental_parameters, const VariableType& type) {
		return std::visit(overload([&](const IntType&)->std::variant<std::vector<TypeError>, std::optional<std::shared_ptr<TypeRegister::TypeInfo>>>{return types.get_for_int();}, [&](const PointerType& t){return instantiate_type(outer_class, environmental_parameters, t);}), type);
	}

	std::variant<std::vector<TypeError>, std::optional<std::shared_ptr<TypeRegister::TypeInfo>>> instantiate_type(const std::optional<std::string>& outer_class, const std::map<std::string, std::shared_ptr<TypeRegister::TypeInfo>>& environmental_parameters, const AstNode& node, const std::string& name, const std::vector<Type>& parameters) {
		std::vector<TypeError> errors;
		if(outer_class && name == *outer_class) {
			if(parameters.empty()) {
				return std::nullopt;
			} else {
				errors.emplace_back(node, "Inside its own definition type may only be used without parameters.");
				return errors;
			}
		}

		{
			auto env_type = environmental_parameters.find(name);
			if(env_type != environmental_parameters.end()) {
				if(parameters.empty()) {
					return env_type->second;
				}
				errors.emplace_back(node, "Template template parameters not supported.");
				return errors;
			}
		}

		std::vector<std::shared_ptr<TypeRegister::TypeInfo>> params;
		for(size_t i = 0; i < parameters.size(); ++i) {
			instantiate_type({}, environmental_parameters, parameters[i], errors, [&](const auto& iter){
				if(iter) {
					params.push_back((*iter));
				} else {
					auto v = [&](const auto& t){errors.emplace_back(t, "Class cannot appear as a template argument in its own definition.");};
					visit_type(parameters[i], v, v, v);
				}
			});
		}

		if(!errors.empty()) {
			return errors;
		}

		{
			size_t i = types.find(name, params);
			if(i < types.size()) {
				return types.at(i);
			}
		}

		if(!patterns.has(name)) {
			errors.emplace_back(node, "Class not found.");
			return errors;
		}

		return std::visit(overload(
		[&](std::vector<TypeError>&& e)->std::variant<std::vector<TypeError>, std::optional<std::shared_ptr<TypeRegister::TypeInfo>>>{
			return move(e);
		},
		[&](std::shared_ptr<TypeRegister::TypeInfo>&& iter)->std::variant<std::vector<TypeError>, std::optional<std::shared_ptr<TypeRegister::TypeInfo>>>{
			return move(iter);
		}
		), instantiate_type_do(patterns[name], params));
	}

	std::variant<std::vector<TypeError>, std::shared_ptr<TypeRegister::TypeInfo>> instantiate_type_do(const Class* class_ast, std::vector<std::shared_ptr<TypeRegister::TypeInfo>> parameters) {
		std::vector<TypeError> errors;
		for(const Identifier& i : class_ast->parameters) {
			if(i.name == class_ast->name.name) {
				errors.emplace_back(i, "Parameter name same as class name.");
			}
		}
		if(class_ast->parameters.size() != parameters.size()) {
			std::string error_msg = "Class instantiated with wrong number of arguments (expected " + std::to_string(class_ast->parameters.size()) + ", got " + std::to_string(parameters.size()) + ").";
			if(parameters.empty()) {
				errors.emplace_back(class_ast->name, std::move(error_msg));
			} else {
				errors.emplace_back(class_ast->parameters[0].begin, class_ast->parameters.rbegin()->begin, std::move(error_msg));
			}
		}
		if(!errors.empty()) {
			return errors;
		}
		std::map<std::string, std::shared_ptr<TypeRegister::TypeInfo>> environmental_parameters;
		for(size_t i = 0; i < parameters.size(); ++i) {
			environmental_parameters.insert(std::make_pair(class_ast->parameters[i].name, parameters[i]));
		}

		std::optional<std::shared_ptr<TypeRegister::TypeInfo>> superclass;

		std::map<std::pair<std::string, size_t>, std::shared_ptr<TypeRegister::FunctionInfo>> inherited_functions;

		auto call_instantiate_type_optional = [&](const auto& type, auto action){
			instantiate_type(class_ast->name.name, environmental_parameters, type, errors, action);
		};

		if(class_ast->superclass){
			call_instantiate_type_optional(*(class_ast->superclass), [&](const auto& iter){
				superclass = iter;
				if(!superclass) {
					errors.emplace_back(*(class_ast->superclass), "Class cannot derive from itself.");
				} else if((**(**superclass).ast_data).public_variables) {
					errors.emplace_back(*(class_ast->superclass), "Cannot extend struct-like classes.");
				}
			});

			if(!errors.empty()) {
				return errors;
			}

			inherited_functions = (**superclass).get_functions();
		}

		std::shared_ptr<TypeRegister::TypeInfo> type_info(std::make_shared<TypeRegister::TypeInfo>(class_ast->name.name, std::move(parameters), std::move(superclass), std::map<std::string, std::shared_ptr<TypeRegister::TypeInfo>>{}, std::move(inherited_functions), std::map<size_t, std::vector<std::shared_ptr<TypeRegister::TypeInfo>>>{}, true, true, class_ast));

		auto call_instantiate_type = [&](const auto& type, std::function<void(std::shared_ptr<TypeRegister::TypeInfo>)> action){
			instantiate_type(class_ast->name.name, environmental_parameters, type, errors, [&](auto iter){
				if(iter) {
					action(*iter);
				} else {
					action(type_info);
				}
			});
		};

		{
			auto add_variable = [&](const ClassVariable& node, std::string name, std::shared_ptr<TypeRegister::TypeInfo> type){
				if(type_info->variables.find(name) != type_info->variables.end()) {
					std::visit([&](const auto& n){errors.emplace_back(n, "Redeclaration of a class variable.");}, node);
				} else {
					type_info->variables.insert(std::make_pair(std::move(name), type));
				}
			};

			for(const ClassVariable& var : class_ast->variables) {
				std::visit(overload(
				[&](const ClassVariableInteger& v){add_variable(var, v.name.name, types.get_for_int());},
				[&](const ClassVariablePointer& v){
					call_instantiate_type(v.type, [&](const auto& t){
						add_variable(var, v.name.name, t);
					});
				}
				), var);
			}
		}

		auto check_argument_declaration = [&](const auto& f) -> std::vector<std::shared_ptr<TypeRegister::TypeInfo>> {
			std::vector<std::shared_ptr<TypeRegister::TypeInfo>> args;
			for(const auto&[type, ident] : f.arguments) {
				call_instantiate_type(type, [&](const auto& t){
					args.push_back(t);
				});
			}
			return args;
		};

		for(const Constructor& c : class_ast->constructors) {
			std::vector<std::shared_ptr<TypeRegister::TypeInfo>> args = check_argument_declaration(c);
			if(args.size() == c.arguments.size()) {
				if(type_info->constructors.find(args.size()) != type_info->constructors.end()) {
					errors.emplace_back(c.begin, c.body->begin, "Constructor overload only possible with a different number of arguments.");
					continue;
				}
				type_info->constructors.insert(std::make_pair(args.size(), move(args)));
			}
		}

		for(const Function& f : class_ast->functions) {
			std::shared_ptr<TypeRegister::TypeInfo> return_type;
			std::vector<std::shared_ptr<TypeRegister::TypeInfo>> args = check_argument_declaration(f);
			if(args.size() == f.arguments.size()) {
				auto te = type_info->insert_function(f.name.name, return_type, move(args), &f);
				if(te) {
					errors.emplace_back(std::move(*te));
				}
			}
		}

		{
			auto te = type_info->check_purely_virtual_functions();
			if(te) {
				for(TypeError& e : *te) {
					errors.emplace_back(std::move(e));
				}
			}
		}

		{
			auto check = [&](const Function& f){
				if(class_ast->nojournal && f.kind) {
					errors.emplace_back(f.name, "Nojournal class must not specify function kind.");
				} else if (!class_ast->nojournal && !f.kind) {
					errors.emplace_back(f.name, "Class with journal must specify function kind.");
				}
			};
			for(const Function& f : class_ast->functions) {
				check(f);
			}
			for(const std::vector<Function>& group : class_ast->optional_functions) {
				for(const Function& f : group) {
					check(f);
				}
			}
		}

		for(const std::vector<Function>& group : class_ast->optional_functions) {
			for(const Function& f : group) {
				if(!f.body) {
					errors.emplace_back(f.name, "Optional functions cannot be purely virtual.");
				}
			}
		}

		if(!errors.empty()) {
			return errors;
		}

		{
			std::map<std::string, std::shared_ptr<TypeRegister::TypeInfo>> variable_map = type_info->variables;
			variable_map[THIS_IDENTIFIER] = type_info;
			std::vector<std::set<std::string>> variable_stack;

			auto add_variable_do = [&](const Identifier& name, std::shared_ptr<TypeRegister::TypeInfo> type){
				if(variable_map.find(name.name) != variable_map.end()) {
					errors.emplace_back(name, "Variable redefinition.");
					return;
				}
				variable_map.insert(std::make_pair(name.name, type));
			};

			auto add_variable = overload(
			add_variable_do,
			[&](const Identifier& name, const auto& type){
				call_instantiate_type(type, [&](auto t){
					add_variable_do(name, t);
				});
			}
			);

			auto remove_last_variable_block = [&](){
				for(const std::string& name : variable_stack[variable_stack.size() - 1]) {
					variable_map.erase(name);
				}
				variable_stack.pop_back();
			};

			std::function<std::optional<ExpressionCheckResult>(const Expression&)> check_expression;

			auto get_type_convertibility_error = [&](const std::string& a, const std::string& b){
				return "Type " + a + " not convertible to " + b + ".";
			};

			check_expression = [&](const Expression& e)->std::optional<ExpressionCheckResult>{
				return std::visit(overload(
				[&](const std::unique_ptr<StringLiteral>& e)->std::optional<ExpressionCheckResult>{
					size_t t = types.find(STRING_TYPE_NAME, {});
					if(t < types.size()) {
						return ExpressionCheckResult(types.at(t), false);
					}
					errors.emplace_back(*e, "String library required to use string literals.");
					return std::nullopt;
				},
				[&](const std::unique_ptr<IntegerLiteral>&)->std::optional<ExpressionCheckResult>{
					return ExpressionCheckResult(types.get_for_int(), false);
				},
				[&](const std::unique_ptr<Identifier>& e)->std::optional<ExpressionCheckResult>{
					auto t = variable_map.find(e->name);
					if(t == variable_map.end()) {
						errors.emplace_back(*e, "Variable not found in current context.");
						return std::nullopt;
					}
					return ExpressionCheckResult(t->second, true);
				},
				[&](const std::unique_ptr<New>& e)->std::optional<ExpressionCheckResult>{
					std::shared_ptr<TypeRegister::TypeInfo> t;
					call_instantiate_type(e->type, [&](auto tt){t = tt;});
					if(!t) {
						return std::nullopt;
					}
					auto constructor = t->constructors.find(e->args.size());
					if(constructor == t->constructors.end()) {
						errors.emplace_back(*e, "Constructor with given number of arguments not found.");
						return std::nullopt;
					}
					bool err = false;
					for(size_t i = 0; i < e->args.size(); ++i) {
						auto type = check_expression(e->args[i]);
						if(type && !type->implicit_convertible_to(constructor->second[i].get())) {
							errors.emplace_back(e->args[i], get_type_convertibility_error(type->full_name(), constructor->second[i]->full_name()));
							err = true;
						}
					}
					if(err) {
						return std::nullopt;
					}
					return ExpressionCheckResult(t, false);
				},
				[&](const std::unique_ptr<Negation>& e)->std::optional<ExpressionCheckResult>{
					auto t = check_expression(e->value);
					if(!t) {
						return std::nullopt;
					}
					auto tt = t->get_member(NEG_FUN_NAME);
					if(!tt) {
						errors.emplace_back(*e, "Type " + t->full_name() + " doesn't have member " + NEG_FUN_NAME);
						return std::nullopt;
					}
					auto result = tt->call_with({});
					if(result) {
						return *result;
					}
					errors.emplace_back(*e, "Type " + tt->full_name() + " cannot be called with ().");
					return std::nullopt;
				},
				[&](const std::unique_ptr<Cast>& e)->std::optional<ExpressionCheckResult>{
					auto et = check_expression(e->value);
					std::shared_ptr<TypeRegister::TypeInfo> tt;
					call_instantiate_type(e->target_type, [&](auto t){tt = t;});
					if(!et || !tt) {
						return std::nullopt;
					}
					if(!et->explicit_convertible_to(tt.get())) {
						errors.emplace_back(*e, "Explicit cast of type " + et->full_name() + " to " + tt->full_name() + " not possible.");
						return std::nullopt;
					}
					return ExpressionCheckResult(tt, false);
				},
				[&](const std::unique_ptr<Null>&)->std::optional<ExpressionCheckResult>{
					return ExpressionCheckResult(types.get_for_null(), false);
				},
				[&](const std::unique_ptr<This>&)->std::optional<ExpressionCheckResult>{
					return ExpressionCheckResult(type_info, false);
				},
				[&](const std::unique_ptr<MemberAccess>& e)->std::optional<ExpressionCheckResult>{
					auto t = check_expression(e->object);
					if(!t) {
						return std::nullopt;
					}
					auto tt = t->get_member(e->member.name);
					if(!tt) {
						errors.emplace_back(*e, "Type " + t->full_name() + " doesn't have member " + e->member.name);
					}
					return tt;
				},
				[&](const std::unique_ptr<FunctionCall>& e)->std::optional<ExpressionCheckResult>{
					auto t = check_expression(e->function);
					std::vector<ExpressionCheckResult> args;
					for(const Expression& ee : e->arguments) {
						auto tt = check_expression(ee);
						if(tt) {
							args.push_back(std::move(*tt));
						}
					}
					if(args.size() != e->arguments.size() || !t) {
						return std::nullopt;
					}
					auto result = t->call_with(args);
					if(result) {
						return *result;
					}
					std::string arg_types;
					if(args.size()) {
						arg_types = args[0].full_name();
					}
					for(size_t i = 1; i < args.size(); ++i) {
						arg_types += ", ";
						arg_types += args[i].full_name();
					}
					errors.emplace_back(*e, "Type " + t->full_name() + " cannot be called with (" + arg_types + ").");
					return std::nullopt;
				},
				[&](const std::unique_ptr<JournaledThisCall>& e)->std::optional<ExpressionCheckResult>{
					auto t = check_expression(e->function);
					std::vector<ExpressionCheckResult> args;
					for(const Expression& ee : e->arguments) {
						auto tt = check_expression(ee);
						if(tt) {
							args.push_back(std::move(*tt));
						}
					}
					if(args.size() != e->arguments.size() || !t) {
						return std::nullopt;
					}
					auto result = t->call_with(args);
					if(result) {
						return *result;
					}
					std::string arg_types;
					if(args.size()) {
						arg_types = args[0].full_name();
					}
					for(size_t i = 1; i < args.size(); ++i) {
						arg_types += ", ";
						arg_types += args[i].full_name();
					}
					errors.emplace_back(*e, "Type " + t->full_name() + " cannot be called with (" + arg_types + ").");
					return std::nullopt;
				},
				[&](const std::unique_ptr<LazyFunctionCall>& e)->std::optional<ExpressionCheckResult>{
					auto t = check_expression(e->function);
					std::vector<ExpressionCheckResult> args;
					for(const auto&[expr, is_non_journal] : e->arguments) {
						auto tt = check_expression(expr);
						if(tt) {
							if(!tt->check_lazy_call_journal_mark(is_non_journal)) {
								if(is_non_journal) {
									errors.emplace_back(expr, "Expression marked as non-journalable for a lazy call, but type supports journaling.");
								} else {
									errors.emplace_back(expr, "Type used in lazy call doesn't support journaling. If you know what you're doing use the ':' mark to silence this error.");
								}
							} else {
								args.push_back(std::move(*tt));
							}
						}
					}
					if(args.size() != e->arguments.size() || !t) {
						return std::nullopt;
					}
					auto result = t->call_with(args);
					if(result) {
						return *result;
					}
					std::string arg_types;
					if(args.size()) {
						arg_types = args[0].full_name();
					}
					for(size_t i = 1; i < args.size(); ++i) {
						arg_types += ", ";
						arg_types += args[i].full_name();
					}
					errors.emplace_back(*e, "Type " + t->full_name() + " cannot be called with (" + arg_types + ").");
					return std::nullopt;
				},
				[&](const std::unique_ptr<BinaryOperation>& e)->std::optional<ExpressionCheckResult>{
					auto left = check_expression(e->left);
					auto right = check_expression(e->right);
					if(!left || !right) {
						return std::nullopt;
					}
					std::string function_name;
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
					if(!function_name.empty()) {
						auto f = left->get_member(function_name);
						if(!f) {
							errors.emplace_back(*e, "Type " + left->full_name() + " doesn't have member " + function_name);
							return std::nullopt;
						}
						auto result = f->call_with({*right});
						if(result) {
							return *result;
						}
						errors.emplace_back(*e, "Type " + f->full_name() + " cannot be called with (" + right->full_name() + ").");
						return std::nullopt;
					} else {
						if(!left->comparable_with(*right)) {
							errors.emplace_back(*e, "Types " + left->full_name() + " and " + right->full_name() + " cannot be compared using the '==' sign.");
						}
						return ExpressionCheckResult(types.get_for_int(), false);
					}
				}
				),e);
			};

			std::function<bool(const TypeRegister::TypeInfo*, const std::optional<std::vector<std::shared_ptr<TypeRegister::TypeInfo>>>&, const Statement&, bool)> check_statement;

			auto check_block = [&](const TypeRegister::TypeInfo* return_type, const std::optional<std::vector<std::shared_ptr<TypeRegister::TypeInfo>>>& dual, const Block& block, bool in_loop){
				bool returns = false;
				variable_stack.emplace_back();
				for(const auto& s : block.statements) {
					bool r = check_statement(return_type, dual, s, in_loop);
					returns |= r;
				}
				remove_last_variable_block();
				return returns;
			};

			check_statement = [&](const TypeRegister::TypeInfo* return_type, const std::optional<std::vector<std::shared_ptr<TypeRegister::TypeInfo>>>& dual, const Statement& statement, bool in_loop){
				return std::visit(overload(
				[&](const std::unique_ptr<Break>& s) {
					if(!in_loop) {
						errors.emplace_back(*s, "Cannot break outside loop.");
					}
					return false;
				},
				[&](const std::unique_ptr<Continue>& s) {
					if(!in_loop) {
						errors.emplace_back(*s, "Cannot continue outside loop.");
					}
					return false;
				},
				[&](const std::unique_ptr<Block>& s) {
					return check_block(return_type, dual, *s, in_loop);
				},
				[&](const std::unique_ptr<Empty>&) {
					return false;
				},
				[&](const std::unique_ptr<StatementExpression>& s) {
					check_expression(s->expression);
					return false;
				},
				[&](const std::unique_ptr<For>& s) {
					variable_stack.emplace_back();
					check_statement(return_type, dual, s->before, in_loop);
					auto condition_type = check_expression(s->condition);
					if(condition_type) {
						if(!condition_type->implicit_convertible_to(types.get_for_int().get())) {
							errors.emplace_back(s->condition, get_type_convertibility_error(condition_type->full_name(), "int"));
						}
					}
					check_statement(return_type, dual, s->after, in_loop);
					check_block(return_type, dual, *s->body, true);
					remove_last_variable_block();
					return false;
				},
				[&](const std::unique_ptr<VariableDeclaration>& s) {
					call_instantiate_type(s->type, [&](auto t){
						add_variable(s->name, t);
						if(s->value) {
							auto v = check_expression(*s->value);
							if(v && !v->implicit_convertible_to(t.get())) {
								errors.emplace_back(*s->value, get_type_convertibility_error(v->full_name(), t->full_name()));
							}
						}
					});
					return false;
				},
				[&](const std::unique_ptr<Return>& s) {
					size_t direct = return_type != types.get_for_void().get();
					size_t dual_args = dual ? dual->size() : 0;
					if(direct + dual_args != s->values.size()) {
						errors.emplace_back(*s, "Wrong number of returned values, expected " + std::to_string(s->values.size()) + ", got " + std::to_string(dual_args + direct) + ".");
						return true;
					}
					if(direct) {
						auto t = check_expression(s->values[0]);
						if(t && !t->implicit_convertible_to(return_type)) {
							errors.emplace_back(s->values[0], get_type_convertibility_error(t->full_name(), return_type->full_name()));
						}
					}
					for(size_t i = direct; i < dual_args; ++i) {
						auto t = check_expression(s->values[i]);
						if(t && !t->implicit_convertible_to((*dual)[i - direct].get())) {
							errors.emplace_back(s->values[i], get_type_convertibility_error(t->full_name(), (*dual)[i - direct]->full_name()));
						}
					}
					return true;
				},
				[&](const std::unique_ptr<If>& s) {
					auto condition_type = check_expression(s->condition);
					if(condition_type) {
						if(!condition_type->implicit_convertible_to(types.get_for_int().get())) {
							errors.emplace_back(s->condition, get_type_convertibility_error(condition_type->full_name(), "int"));
						}
					}
					bool returns = check_block(return_type, dual, *s->body, in_loop);
					if(s->body_else) {
						returns &= check_block(return_type, dual, **s->body_else, in_loop);
					} else {
						returns = false;
					}
					return returns;
				},
				[&](const std::unique_ptr<While>& s) {
					auto condition_type = check_expression(s->condition);
					if(condition_type) {
						if(!condition_type->implicit_convertible_to(types.get_for_int().get())) {
							errors.emplace_back(s->condition, get_type_convertibility_error(condition_type->full_name(), "int"));
						}
					}
					check_block(return_type, dual, *s->body, true);
					return false;
				},
				[&](const std::unique_ptr<Assignment>& s) {
					auto var = check_expression(s->variable);
					auto val = check_expression(s->value);
					if(var && val) {
						if(!var->is_lvalue()) {
							errors.emplace_back(*s, "Expected lvalue as left side of assignment.");
						}
						if(!val->implicit_convertible_to(*var)) {
							errors.emplace_back(*s, get_type_convertibility_error(var->full_name(), val->full_name()));
						}
					}
					return false;
				}
				), statement);
			};

			for(const auto& [meta, f] : type_info->own_functions) {
				variable_stack.emplace_back();
				if(!f->ast) {
					throw std::runtime_error("Internal error.");
				}
				for(const auto& [type, name] : (**f->ast).arguments) {
					add_variable(name, type);
				}
				if((**f->ast).body) {
					size_t error_count = errors.size();
					std::optional<std::vector<std::shared_ptr<TypeRegister::TypeInfo>>> dual_call;
					if((**f->ast).dual) {
						dual_call = std::vector<std::shared_ptr<TypeRegister::TypeInfo>>{};
						for(const auto& [type, name] : (**f->ast).dual->first) {
							call_instantiate_type(type, [&](auto t) {
								dual_call->push_back(t);
							});
						}
					}
					if(errors.size() == error_count) {
						bool returns = check_block(f->return_type.get(), dual_call, **(**f->ast).body, false);
						if(!returns && (f->return_type != types.get_for_void() || ((**f->ast).dual && !(**f->ast).dual->first.empty()))) {
							errors.emplace_back((**f->ast).name, "Non-void function doesn't return a value in some cases.");
						}
					}
				}
				remove_last_variable_block();
				if((**type_info->ast_data).nojournal) {
					if((**f->ast).kind) {
						errors.emplace_back((**f->ast).name, "Functions inside classes without journals must not specify their kind.");
					}
				} else {
					if(!(**f->ast).kind) {
						errors.emplace_back((**f->ast).name, "Functions inside classes with journals must specify their kind.");
					}
				}
				if((**f->ast).dual) {
					variable_stack.emplace_back();
					for(const auto& [type, name] : (**f->ast).dual->first) {
						add_variable(name, type);
					}
					check_block(types.get_for_void().get(), std::nullopt, *(**f->ast).dual->second, false);
				}
				remove_last_variable_block();
			}
		}

#warning check_constructors

#warning add_optional_functions

#warning check_variable_function_overload

		if(!types.add(type_info)) {
			throw std::runtime_error("Internal error.");
		}

		return type_info;
	}
};

#endif
