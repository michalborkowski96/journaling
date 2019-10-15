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

constexpr const char* const ADD_FUN_NAME = "add";
constexpr const char* const SUB_FUN_NAME = "sub";
constexpr const char* const MUL_FUN_NAME = "mul";
constexpr const char* const DIV_FUN_NAME = "div";
constexpr const char* const OR_FUN_NAME = "or";
constexpr const char* const AND_FUN_NAME = "and";
constexpr const char* const NEG_FUN_NAME = "neg";
constexpr const char* const OPP_FUN_NAME = "opp";
constexpr const char* const MOD_FUN_NAME = "mod";
constexpr const char* const LESS_FUN_NAME = "less";
constexpr const char* const LESS_EQUAL_FUN_NAME = "le";
constexpr const char* const MORE_FUN_NAME = "more";
constexpr const char* const MORE_EQUAL_FUN_NAME = "me";

constexpr const char* const STRING_TYPE_NAME = "String";
constexpr const char* const INT_TYPE_NAME = "int";
constexpr const char* const NULL_TYPE_NAME = "null";
constexpr const char* const VOID_TYPE_NAME = "void";

constexpr const char* const THIS_NAME = "this";

class TypeError {
	size_t begin;
	size_t end;
	std::string data;
	std::optional<std::shared_ptr<const std::string>> filename;
	std::vector<std::shared_ptr<const std::string>> context;
public:
	TypeError() = delete;
	TypeError(size_t, size_t, std::string);
	TypeError(const AstNode&, std::string);
	TypeError(const Expression&, std::string);
	TypeError(const Statement&, std::string);
	TypeError(const Type&, std::string);
	TypeError(const VariableType&, std::string);
	void fill_filename_if_empty(std::shared_ptr<const std::string> fname);
	void add_context(std::shared_ptr<const std::string> ctx);
};

TypeError::TypeError(size_t begin, size_t end, std::string data) : begin(begin), end(end), data(move(data)) {}

TypeError::TypeError(const AstNode& node, std::string data) : begin(node.begin), end(node.end), data(move(data)) {}

TypeError::TypeError(const Expression& node, std::string data) : begin(std::visit([](const auto& p){return p->begin;}, node)), end(std::visit([](const auto& p){return p->end;}, node)), data(move(data)) {}

TypeError::TypeError(const Statement& node, std::string data) : begin(std::visit([](const auto& p){return p->begin;}, node)), end(std::visit([](const auto& p){return p->end;}, node)), data(move(data)) {}

TypeError::TypeError(const Type& node, std::string data) : begin(std::visit(overload([](const VariableType& p){return std::visit([](const auto& p){return p.begin;}, p);}, [](const auto& p){return p.begin;}), node)), end(std::visit(overload([](const VariableType& p){return std::visit([](const auto& p){return p.end;}, p);}, [](const auto& p){return p.end;}), node)), data(move(data)) {}

TypeError::TypeError(const VariableType& node, std::string data) : begin(std::visit([](const auto& p){return p.begin;}, node)), end(std::visit([](const auto& p){return p.end;}, node)), data(move(data)) {}

void TypeError::fill_filename_if_empty(std::shared_ptr<const std::string> fname) {
	filename = fname;
}

void TypeError::add_context(std::shared_ptr<const std::string> ctx){
	context.push_back(ctx);
}

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

void call_with_error_log(std::vector<TypeError>& errors, std::optional<std::vector<TypeError>>&& err){
	if(err) {
		for(const TypeError& e : *err) {
			errors.push_back(std::move(e));
		}
	}
}

template<typename U>
void call_with_error_log(std::vector<TypeError>& errors, std::optional<std::vector<TypeError>>&& err, U action_success){
	if(err) {
		for(const TypeError& e : *err) {
			errors.push_back(std::move(e));
		}
	} else {
		action_success();
	}
}

class TypeInfo;
class RealClassInfo;
class NullTypeInfo;
class IntTypeInfo;
class TemplateUnknownTypeInfo;
class VoidTypeInfo;
class FunctionalTypeInfo;
class FunctionInfo;

class ClassDatabase {
	std::map<std::string, std::map<std::vector<const TypeInfo*>, std::unique_ptr<const TypeInfo>>> types; //pattern name -> params -> type_info
	std::map<std::string, const TypeInfo*> current_class_parameters;
	std::map<std::string, std::pair<const Class*, std::shared_ptr<const std::string>>> patterns; //name -> ast, filename
	std::optional<const TypeInfo*> get(const std::string& full_name) const;
	std::vector<TypeError> optional_errors;

public:
	class NonOwnedParameter {
		friend class ClassDatabase;
		ClassDatabase* cd;
		std::string name;
		NonOwnedParameter(ClassDatabase* cd, std::string name, const TypeInfo* type);
	public:
		NonOwnedParameter() = delete;
		NonOwnedParameter(const NonOwnedParameter&) = delete;
		NonOwnedParameter(NonOwnedParameter&& o) : cd(o.cd), name(move(o.name)) {
			o.cd = nullptr;
		}
		~NonOwnedParameter();
	};
	friend class NonOwnedParameter;
	ClassDatabase();
	ClassDatabase(const ClassDatabase&) = delete;
	ClassDatabase(ClassDatabase&&) = delete;
	const TypeInfo* get_for_null() const {
		return types.at(NULL_TYPE_NAME).at({}).get();
	}
	std::optional<std::vector<TypeError>> insert_pattern(const Class* p, std::shared_ptr<const std::string> filename);
	const TypeInfo* get_for_int() const {
		return types.at(INT_TYPE_NAME).at({}).get();
	}
	const TypeInfo* get_for_void() const {
		return types.at(VOID_TYPE_NAME).at({}).get();
	}
	std::optional<const TypeInfo*> get_for_string() const {
		return get(STRING_TYPE_NAME);
	}
	std::variant<const TypeInfo*, std::vector<TypeError>> get(const Identifier& t) {
		auto r = get(t.name);
		if(r) {
			return *r;
		}
		return std::vector<TypeError>{TypeError(t, "Class instance not found.")};
	}
	std::variant<const TypeInfo*, std::vector<TypeError>> get(const Type& t) {
		return std::visit(overload(
		[&](const VoidType&)->std::variant<const TypeInfo*, std::vector<TypeError>>{return get_for_void();},
		[&](const VariableType& tt) {return get(tt);}
		), t);
	}
	std::variant<const TypeInfo*, std::vector<TypeError>> get(const VariableType& t);
	std::variant<const TypeInfo*, std::vector<TypeError>> get(const PointerType& t);
	[[nodiscard]] NonOwnedParameter add_parameter(std::string name, const TypeInfo*);
};

std::string types_to_string(const std::vector<const TypeInfo*>& v);

class ExpressionCheckResult {
	const TypeInfo* type;
	const bool lvalue;
public:
	ExpressionCheckResult() = delete;
	ExpressionCheckResult(const TypeInfo* type, bool lvalue) : type(type), lvalue(lvalue) {}
	const TypeInfo* type_info() const {
		return type;
	}
	bool is_lvalue() const {
		return lvalue;
	}
};

class FunctionInfo {
protected:
	FunctionInfo() = default;
public:
	FunctionInfo(const FunctionInfo&) = delete;
	FunctionInfo(FunctionInfo&&) = delete;
	virtual const TypeInfo* type_info() const = 0;
	virtual std::optional<FunctionKind> kind() const = 0;
	virtual std::optional<const TypeInfo*> call_with(const std::vector<const TypeInfo*>& args) const = 0;
	virtual std::string full_name() const = 0;
	virtual ~FunctionInfo() = default;
};

class BuiltinIntFunctionInfo : public FunctionInfo {
protected:
	const TypeInfo* int_type;
	const char* name;
	bool arg;
	const std::unique_ptr<TypeInfo> type_info_data;
public:
	BuiltinIntFunctionInfo() = delete;
	BuiltinIntFunctionInfo(const TypeInfo* int_type, const char* name, bool arg);
	const TypeInfo* type_info() const override {
		return type_info_data.get();
	}
	virtual std::optional<FunctionKind> kind() const override {
		return FunctionKind::NOEFFECT;
	}
	virtual std::optional<const TypeInfo*> call_with(const std::vector<const TypeInfo*>& args) const override {
		if(arg) {
			if(args == std::vector{int_type}) {
				return int_type;
			} else {
				return std::nullopt;
			}
		} else {
			if(args.empty()) {
				return int_type;
			} else {
				return std::nullopt;
			}
		}
	}
	virtual std::string full_name() const override {
		return arg ? std::string("int ") + name + "(int)" : std::string("int ") + name + "()";
	}
	virtual ~BuiltinIntFunctionInfo() = default;
};

class RealFunctionInfo : public FunctionInfo {
	const std::string name;
	const TypeInfo* return_type;
	const std::unique_ptr<TypeInfo> type_info_data;
	const std::vector<const TypeInfo*> arguments;
	const Function* declaration_ast;
	const std::optional<const Function*> body_ast;
	const std::optional<const std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<statement::Block>>*> dual_ast;
	RealFunctionInfo(std::string name, const TypeInfo* return_type, std::vector<const TypeInfo*> arguments, const Function* declaration_ast, std::optional<const Function*> body_ast, std::optional<const std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<statement::Block>>*> dual_ast);
public:
	RealFunctionInfo() = delete;
	const TypeInfo* type_info() const override {
		return type_info_data.get();
	}

	static std::variant<std::unique_ptr<RealFunctionInfo>, std::vector<TypeError>> make_function_declaration(std::optional<const RealFunctionInfo*> overridden, ClassDatabase& class_database, const Function* ast);
	virtual std::optional<FunctionKind> kind() const override {
		return declaration_ast->kind;
	}
	virtual std::optional<const TypeInfo*> call_with(const std::vector<const TypeInfo*>& args) const override;
	bool fully_defined() const {
		if(kind() && (*kind()) == FunctionKind::DUAL) {
			return body_ast && dual_ast;
		} else {
			return (bool) body_ast;
		}
	}
	virtual std::string full_name() const override;
	std::optional<std::vector<TypeError>> check_body(std::map<std::string, const TypeInfo*> variable_map, ClassDatabase& class_database) const;
	virtual ~RealFunctionInfo() = default;
};

class TypeInfo {
	friend class RealClassInfo;
	friend class NullTypeInfo;
	friend class IntTypeInfo;
	friend class TemplateUnknownTypeInfo;
	friend class VoidTypeInfo;
	friend class FunctionalTypeInfo;
	virtual bool implicitly_convertible_from(const RealClassInfo* o) const = 0;
	virtual bool implicitly_convertible_from(const NullTypeInfo* o) const = 0;
	virtual bool implicitly_convertible_from(const IntTypeInfo* o) const = 0;
	virtual bool implicitly_convertible_from(const TemplateUnknownTypeInfo* o) const = 0;
	bool implicitly_convertible_from(const VoidTypeInfo*) const {
		return false;
	}
	bool implicitly_convertible_from(const FunctionalTypeInfo*) const {
		return false;
	}
	TypeInfo() = default;
	TypeInfo(const TypeInfo&) = delete;
	TypeInfo(TypeInfo&&) = delete;
public:
	virtual bool extendable() const {
		return false;
	}
	virtual std::optional<const TypeInfo*> get_superclass() const {
		return std::nullopt;
	}
	virtual bool constructible_with(const std::vector<const TypeInfo*>&) const {
		return false;
	}
	virtual std::optional<const TypeInfo*> call_with(const std::vector<const TypeInfo*>&) const {
		return std::nullopt;
	}
	virtual bool check_nojournal_mark(bool mark) const = 0;
	virtual bool implicitly_convertible_to(const TypeInfo* o) const = 0;
	virtual std::string full_name() const = 0;
	virtual std::optional<const TypeInfo*> get_member(const std::string& name) const = 0;

	virtual ~TypeInfo() = default;

	bool explicitly_convertible_to(const TypeInfo* o) const {
		return implicitly_convertible_to(o) || o->implicitly_convertible_to(this);
	}

	bool comparable_with(const TypeInfo* o) const {
		return explicitly_convertible_to(o);
	}
};

class FunctionalTypeInfo : public TypeInfo {
	const FunctionInfo* fi;
	virtual bool implicitly_convertible_from(const RealClassInfo*) const override {
		return false;
	}
	virtual bool implicitly_convertible_from(const NullTypeInfo*) const override {
		return false;
	}
	virtual bool implicitly_convertible_from(const IntTypeInfo*) const override {
		return false;
	}
	virtual bool implicitly_convertible_from(const TemplateUnknownTypeInfo*) const override {
		return false;
	}
	FunctionalTypeInfo() = default;
public:
	FunctionalTypeInfo(const FunctionInfo* fi) : fi(fi) {}
	virtual bool check_nojournal_mark(bool) const override {
		return true;
	}
	virtual bool implicitly_convertible_to(const TypeInfo* o) const override {
		return o->implicitly_convertible_from(this);
	}
	virtual std::string full_name() const override {
		return fi->full_name();
	}
	virtual std::optional<const TypeInfo*> get_member(const std::string&) const override {
		return std::nullopt;
	}
	virtual std::optional<const TypeInfo*> call_with(const std::vector<const TypeInfo*>& args) const override {
		return fi->call_with(args);
	}
	virtual ~FunctionalTypeInfo() = default;
};

class NullTypeInfo : public TypeInfo {
	virtual bool implicitly_convertible_from(const RealClassInfo*) const override {
		return false;
	}
	virtual bool implicitly_convertible_from(const NullTypeInfo*) const override {
		return false;
	}
	virtual bool implicitly_convertible_from(const IntTypeInfo*) const override {
		return false;
	}
	virtual bool implicitly_convertible_from(const TemplateUnknownTypeInfo*) const override {
		return false;
	}
public:
	virtual bool check_nojournal_mark(bool mark) const override {
		return !mark;
	}
	virtual bool implicitly_convertible_to(const TypeInfo* o) const override {
		return o->implicitly_convertible_from(this);
	}
	virtual std::string full_name() const override {
		return NULL_TYPE_NAME;
	}
	virtual std::optional<const TypeInfo*> get_member(const std::string&) const override {
		return std::nullopt;
	}
	virtual ~NullTypeInfo() = default;
};

class VoidTypeInfo : public TypeInfo {
	virtual bool implicitly_convertible_from(const RealClassInfo*) const override {
		return false;
	}
	virtual bool implicitly_convertible_from(const NullTypeInfo*) const override {
		return false;
	}
	virtual bool implicitly_convertible_from(const IntTypeInfo*) const override {
		return false;
	}
	virtual bool implicitly_convertible_from(const TemplateUnknownTypeInfo*) const override {
		return false;
	}
public:
	virtual bool check_nojournal_mark(bool mark) const override {
		return !mark;
	}
	virtual bool implicitly_convertible_to(const TypeInfo* o) const override {
		return o->implicitly_convertible_from(this);
	}
	virtual std::string full_name() const override {
		return VOID_TYPE_NAME;
	}
	virtual std::optional<const TypeInfo*> get_member(const std::string&) const override {
		return std::nullopt;
	}
	virtual ~VoidTypeInfo() = default;
};

class IntTypeInfo : public TypeInfo {
	const std::map<std::string, std::unique_ptr<BuiltinIntFunctionInfo>> funs;
	virtual bool implicitly_convertible_from(const RealClassInfo*) const override {
		return false;
	}
	virtual bool implicitly_convertible_from(const NullTypeInfo*) const override {
		return true;
	}
	virtual bool implicitly_convertible_from(const IntTypeInfo*) const override {
		return true;
	}
	virtual bool implicitly_convertible_from(const TemplateUnknownTypeInfo*) const override {
		return false;
	}
	static std::map<std::string, std::unique_ptr<BuiltinIntFunctionInfo>> get_funs_map(const IntTypeInfo* int_type) {
		std::map<std::string, std::unique_ptr<BuiltinIntFunctionInfo>> funs;
		auto ins = [&](const char* name, bool arg){
			funs.emplace(std::string(name), std::make_unique<BuiltinIntFunctionInfo>(int_type, name, arg));
		};
		ins(ADD_FUN_NAME, true);
		ins(SUB_FUN_NAME, true);
		ins(MUL_FUN_NAME, true);
		ins(DIV_FUN_NAME, true);
		ins(OR_FUN_NAME, true);
		ins(AND_FUN_NAME, true);
		ins(NEG_FUN_NAME, false);
		ins(OPP_FUN_NAME, false);
		ins(MOD_FUN_NAME, true);
		ins(LESS_FUN_NAME, true);
		ins(LESS_EQUAL_FUN_NAME, true);
		ins(MORE_FUN_NAME, true);
		ins(MORE_EQUAL_FUN_NAME, true);
		return funs;
	}
public:
	IntTypeInfo() : funs(get_funs_map(this)) {}
	virtual bool check_nojournal_mark(bool mark) const override {
		return !mark;
	}
	virtual bool implicitly_convertible_to(const TypeInfo* o) const override {
		return o->implicitly_convertible_from(this);
	}
	virtual std::string full_name() const override {
		return INT_TYPE_NAME;
	}
	virtual std::optional<const TypeInfo*> get_member(const std::string& name) const override {
		auto it = funs.find(name);
		if(it == funs.end()) {
			return std::nullopt;
		}
		return it->second->type_info();
	}
	virtual ~IntTypeInfo() = default;
};

class TemplateUnknownTypeInfo : public TypeInfo {
	std::string name;
	virtual bool implicitly_convertible_from(const RealClassInfo*) const override {
		return true;
	}
	virtual bool implicitly_convertible_from(const NullTypeInfo*) const override {
		return true;
	}
	virtual bool implicitly_convertible_from(const IntTypeInfo*) const override {
		return true;
	}
	virtual bool implicitly_convertible_from(const TemplateUnknownTypeInfo*) const override {
		return true;
	}
public:
	virtual bool constructible_with(const std::vector<const TypeInfo*>&) const override {
		return true;
	}
	virtual bool check_nojournal_mark(bool) const override {
		return true;
	}
	virtual bool implicitly_convertible_to(const TypeInfo* o) const override {
		return o->implicitly_convertible_from(this);
	}
	virtual std::string full_name() const override {
		return name;
	}
	virtual std::optional<const TypeInfo*> get_member(const std::string&) const override {
		return this;
	}
	TemplateUnknownTypeInfo() = delete;
	TemplateUnknownTypeInfo(std::string name) : name(std::move(name)) {}
	virtual ~TemplateUnknownTypeInfo() = default;
};

std::optional<ExpressionCheckResult> check_expression(std::vector<TypeError>& errors, const std::map<std::string, const TypeInfo*>& variable_map, ClassDatabase& class_database, const Expression& e);
std::optional<std::vector<TypeError>> check_function_block(const TypeInfo* return_type, const std::optional<std::vector<const TypeInfo*>>& dual, std::map<std::string, const TypeInfo*>& variable_map, ClassDatabase& class_database, const Block& block);

class RealClassInfo : public TypeInfo {
	std::vector<const TypeInfo*> parameters;
	std::optional<const TypeInfo*> superclass;
	std::map<std::string, const TypeInfo*> variables;
	std::map<std::string, std::unique_ptr<RealFunctionInfo>> functions;
	std::map<size_t, std::vector<const TypeInfo*>> constructors;
	const Class* ast_data;
	RealClassInfo() = delete;
	RealClassInfo(decltype(parameters) parameters, decltype(superclass) superclass, decltype(ast_data) ast_data) : parameters(move(parameters)), superclass(move(superclass)), ast_data(ast_data) {}

	virtual bool implicitly_convertible_from(const RealClassInfo* o) const override {
		if(this == o) {
			return true;
		}
		const TypeInfo* superclass = o;
		while(superclass->get_superclass()) {
			superclass = *superclass->get_superclass();
			if(superclass == this) {
				return true;
			}
		}
		return false;
	}
	virtual bool implicitly_convertible_from(const NullTypeInfo*) const override {
		return true;
	}
	virtual bool implicitly_convertible_from(const IntTypeInfo*) const override {
		return false;
	}
	virtual bool implicitly_convertible_from(const TemplateUnknownTypeInfo*) const override {
		return true;
	}
public:
	virtual std::optional<const TypeInfo*> get_superclass() const override {
		return superclass;
	}
	virtual ~RealClassInfo() = default;
	virtual bool constructible_with(const std::vector<const TypeInfo*>& args) const override {
		auto constructor = constructors.find(args.size());
		if(constructor == constructors.end()) {
			return false;
		}
		for(size_t i = 0; i < args.size(); ++i) {
			if(!args[i]->implicitly_convertible_to(constructor->second[i])) {
				return false;
			}
		}
		return true;
	}
	const std::string& pattern_name() const {
		return ast_data->name.name;
	}
	virtual std::optional<const TypeInfo*> get_member(const std::string& name) const override {
		if(ast_data->public_variables) {
			auto v = variables.find(name);
			if(v != variables.end()) {
				return v->second;
			}
		}

		auto f = get_function(name);
		if(f) {
			return (**f).type_info();
		}
		return std::nullopt;
	}
	virtual bool check_nojournal_mark(bool mark) const override {
		return mark == ast_data->nojournal;
	}
	virtual bool extendable() const override {
		return !ast_data->public_variables;
	}
	static std::variant<std::pair<std::unique_ptr<RealClassInfo>, std::vector<TypeError>>, std::vector<TypeError>> make_class(ClassDatabase& class_database, const Class* ast) {
		std::vector<TypeError> errors;
		std::unique_ptr<RealClassInfo> type_info;
		{
			std::optional<const TypeInfo*> superclass;
			if(ast->superclass) {
				call_with_error_log(errors, class_database.get(*ast->superclass), [&](const TypeInfo* type){
					if(!type->extendable()) {
						errors.emplace_back(*(ast->superclass), "Cannot extend struct-like classes.");
					} else {
						superclass = type;
					}
				});
			}

			std::vector<const TypeInfo*> parameters;

			for(const Identifier& t : ast->parameters) {
				call_with_error_log(errors, class_database.get(t), [&](const TypeInfo* type) {
					parameters.push_back(type);
				});
			}

			if(!errors.empty()) {
				return errors;
			}

			type_info = std::unique_ptr<RealClassInfo>(new RealClassInfo(move(parameters), superclass, ast));
		}

		auto temp = class_database.add_parameter(ast->name.name, type_info.get());

		{
			auto add_variable = [&](const ClassVariable& node, std::string name, const TypeInfo* type){
				if(type_info->variables.find(name) != type_info->variables.end()) {
					std::visit([&](const auto& n){errors.emplace_back(n, "Redeclaration of a class variable.");}, node);
				} else {
					type_info->variables.emplace(std::move(name), type);
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

		auto verify_function_declaration = [&](bool optional, const Function& f, const RealFunctionInfo& fi)->std::optional<std::vector<TypeError>>{
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
			auto check_argument_declaration = [&](const auto& f) -> std::vector<const TypeInfo*> {
				std::vector<const TypeInfo*> args;
				for(const auto&[type, ident] : f.arguments) {
					call_with_error_log(errors, class_database.get(type), [&](const auto& t){
						args.push_back(t);
					});
				}
				return args;
			};

			for(const Constructor& c : ast->constructors) {
				std::vector<const TypeInfo*> args = check_argument_declaration(c);
				if(args.size() == c.arguments.size()) {
					if(type_info->constructors.find(args.size()) != type_info->constructors.end()) {
						errors.emplace_back(c.begin, c.body->begin, "Constructor overload only possible with a different number of arguments.");
					} else {
						type_info->constructors.emplace(args.size(), move(args));
					}
				}
			}

			for(const Function& f : ast->functions) {
				std::optional<const RealFunctionInfo*> overridden;
				if(type_info->superclass && dynamic_cast<const RealClassInfo*>(*type_info->superclass)) {
					overridden = dynamic_cast<const RealClassInfo*>(*type_info->superclass)->get_function(f.name.name);
				}
				call_with_error_log(errors, RealFunctionInfo::make_function_declaration(overridden, class_database, &f), [&](std::unique_ptr<RealFunctionInfo>&& fi){
					call_with_error_log(errors, verify_function_declaration(false, f, *fi), [&](){type_info->functions.emplace(f.name.name, std::move(fi));});
				});
			}
			if(!errors.empty()) {
				return errors;
			}
		}

		std::map<std::string, const TypeInfo*> base_variables;

		{
			const TypeInfo* bs = type_info.get();
			base_variables.emplace(std::string(THIS_NAME), bs);
		}

		for(const auto&[name, t] : type_info->variables) {
			if(base_variables.find(name) != base_variables.end()) {
				throw std::runtime_error("Internal error.");
			}
			base_variables.emplace(name, t);
		}

		for(const auto&[name, f] : type_info->functions) {
			if(base_variables.find(name) != base_variables.end()) {
				throw std::runtime_error("Internal error.");
			}
			base_variables.emplace(name, f->type_info());
		}

		for(const auto&[name, f] : type_info->functions) {
			call_with_error_log(errors, f->check_body(base_variables, class_database));
		}

		auto check_constructor_body = [&errors, &base_variables, &class_database, &superclass = type_info->superclass](const Constructor& ast, const std::vector<const TypeInfo*>& args){
			auto variable_map = base_variables;
			std::map<std::string, const TypeInfo*> superclass_call_variable_map;
			auto add_variable = [&](const Identifier& name, const TypeInfo* type){
				if(variable_map.find(name.name) != variable_map.end()) {
					errors.emplace_back(name, "Variable redefinition.");
					return;
				}
				variable_map.emplace(name.name, type);
				superclass_call_variable_map.emplace(name.name, type);
			};
			for(size_t i = 0; i < args.size(); ++i) {
				add_variable(ast.arguments[i].second, args[i]);
			}
			if(superclass) {
				const TypeInfo* s = *superclass;
				if(!ast.superclass_call) {
					errors.emplace_back(ast.begin, ast.body->begin, "Constructor of the derived class doesn't call superclass constructor.");
				} else {
					std::vector<const TypeInfo*> superclass_call_args;
					for(const Expression& e : *ast.superclass_call) {
						auto t = check_expression(errors, superclass_call_variable_map, class_database, e);
						if(t) {
							superclass_call_args.push_back(t->type_info());
						}
					}
					if(s) {
						if(superclass_call_args.size() == ast.superclass_call->size() && !s->constructible_with(superclass_call_args)){
							errors.emplace_back(ast.begin, ast.body->begin, "Superclass cannot be constructed with arguments " + types_to_string(args) + ".");
						}
					}
				}
			} else {
				if(ast.superclass_call) {
					errors.emplace_back(ast.begin, ast.body->begin, "Constructor calls superclass constructor, but this class has no base class.");
				}
			}
			call_with_error_log(errors, check_function_block(class_database.get_for_void(), std::nullopt, variable_map, class_database, *ast.body));
		};

		for(const Constructor& c : ast->constructors) {
			check_constructor_body(c, type_info->constructors.at(c.arguments.size()));
		}
		std::vector<TypeError> optional_errors;
		for(const std::vector<Function>& funs : ast->optional_functions) {
			size_t errors_size = optional_errors.size();
			std::map<std::string, std::unique_ptr<RealFunctionInfo>> optional_functions;
			for(const Function& f : funs) {
				std::optional<const RealFunctionInfo*> overridden;
				if(type_info->superclass && dynamic_cast<const RealClassInfo*>(*type_info->superclass)) {
					overridden = dynamic_cast<const RealClassInfo*>(*type_info->superclass)->get_function(f.name.name);
				}
				call_with_error_log(optional_errors, RealFunctionInfo::make_function_declaration(overridden, class_database, &f), [&](std::unique_ptr<RealFunctionInfo>&& fi){
					call_with_error_log(optional_errors, verify_function_declaration(true, f, *fi), [&](){
						if(optional_functions.find(f.name.name) != optional_functions.end()) {
							optional_errors.emplace_back(f.name, "Optional function shadows another optional function in same group.");
						} else {
							optional_functions.emplace(f.name.name, std::move(fi));
						}
					});
				});
			}
			if(optional_errors.size() != errors_size) {
				continue;
			}
			for(auto& [name, ptr] : optional_functions) {
				type_info->functions.emplace(name, std::move(ptr));
			}
			for(const auto& [fname, null_ptr] : optional_functions) {
				const auto&[name, f] = *type_info->functions.find(fname);
				if(base_variables.find(name) != base_variables.end()) {
					throw std::runtime_error("Internal error.");
				}
				base_variables.emplace(name, f->type_info());
			}
			for(const auto& [fname, null_ptr] : optional_functions) {
				const auto&[name, f] = *type_info->functions.find(fname);
				call_with_error_log(optional_errors, f->check_body(base_variables, class_database));
			}
			if(optional_errors.size() != errors_size) {
				for(const auto& [fname, null_ptr] : optional_functions) {
					const auto&[name, f] = *type_info->functions.find(fname);
					base_variables.erase(name);
					type_info->functions.erase(name);
				}
			}
		}
		if(!errors.empty()) {
			return errors;
		}
		return std::make_pair(move(type_info), move(optional_errors));
	}
	std::optional<const RealFunctionInfo*> get_function(const std::string& name) const {
		auto f = functions.find(name);
		if(f == functions.end()) {
			return std::nullopt;
		}
		return f->second.get();
	}
	virtual bool implicitly_convertible_to(const TypeInfo* o) const override {
		return o->implicitly_convertible_from(this);
	}
	virtual std::string full_name() const override {
		std::string ret = ast_data->name.name;
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

std::string types_to_string(const std::vector<const TypeInfo*>& v) {
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

std::variant<std::unique_ptr<RealFunctionInfo>, std::vector<TypeError>> RealFunctionInfo::make_function_declaration(std::optional<const RealFunctionInfo*> overridden, ClassDatabase& class_database, const Function* ast) {
	std::vector<TypeError> errors;
	const TypeInfo* rt;
	std::vector<const TypeInfo*> args;
	call_with_error_log(errors, class_database.get(ast->return_type), [&](const TypeInfo* type){
		rt = type;
	});
	for(const auto& [type, ident] : ast->arguments) {
		call_with_error_log(errors, class_database.get(type), [&](const TypeInfo* type){
			args.push_back(type);
		});
	}

	std::optional<const Function*> body;
	if(ast->body) {
		body = ast;
	} else if(overridden && (**overridden).body_ast) {
		body = (**overridden).declaration_ast;
	}
	std::optional<const std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<statement::Block>>*> dual;
	if(ast->dual) {
		dual = &*ast->dual;
	} else if(overridden) {
		dual = (**overridden).dual_ast;
	}

	if(overridden) {
		const RealFunctionInfo& fi = **overridden;
		if(fi.arguments != args) {
			errors.emplace_back(ast->name, "Function overrides another function, but has different arguments, " + types_to_string(fi.arguments) + " instead of " + types_to_string(args) + ".");
		}
		if(!rt->implicitly_convertible_to(fi.return_type)) {
			errors.emplace_back(ast->name, "Function overrides another function, but its return type " + rt->full_name() + " isn't implicitly convertible to " + fi.return_type->full_name() + ".");
		}
		if(ast->kind != fi.kind()) {
			errors.emplace_back(ast->name, "Function overrides another function, but has different kind.");
		}
	}

	if(ast->kind == FunctionKind::DUAL && dual && !body) {
		errors.emplace_back(ast->name, "Dual function has primary definition, but no dual definition.");
	}

	if(!errors.empty()) {
		return errors;
	} else {
		return std::unique_ptr<RealFunctionInfo>(new RealFunctionInfo(ast->name.name, std::move(rt), move(args), ast, body, dual));
	}
}

std::string RealFunctionInfo::full_name() const {
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

BuiltinIntFunctionInfo::BuiltinIntFunctionInfo(const TypeInfo* int_type, const char* name, bool arg) : int_type(int_type), name(name), arg(arg), type_info_data(std::make_unique<FunctionalTypeInfo>(this)) {}

RealFunctionInfo::RealFunctionInfo(std::string name, const TypeInfo* return_type, std::vector<const TypeInfo*> arguments, const Function* declaration_ast, std::optional<const Function*> body_ast, std::optional<const std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<statement::Block>>*> dual_ast) : name(move(name)), return_type(std::move(return_type)), type_info_data(std::make_unique<FunctionalTypeInfo>(this)), arguments(move(arguments)), declaration_ast(declaration_ast), body_ast(body_ast), dual_ast(dual_ast) {}

std::optional<ExpressionCheckResult> check_expression(std::vector<TypeError>& errors, const std::map<std::string, const TypeInfo*>& variable_map, ClassDatabase& class_database, const Expression& e) {
	auto check_expr = [&](const Expression& e) {
		return check_expression(errors, variable_map, class_database, e);
	};
	return std::visit(overload(
	[&](const std::unique_ptr<StringLiteral>& e)->std::optional<ExpressionCheckResult>{
		auto t = class_database.get_for_string();
		if(t) {
			return ExpressionCheckResult(*t, false);
		}
		errors.emplace_back(*e, "String library required to use string literals.");
		return std::nullopt;
	},
	[&](const std::unique_ptr<IntegerLiteral>&)->std::optional<ExpressionCheckResult>{
		return ExpressionCheckResult(class_database.get_for_int(), false);
	},
	[&](const std::unique_ptr<Identifier>& e)->std::optional<ExpressionCheckResult>{
		auto t = variable_map.find(e->name);
		if(t == variable_map.end()) {
			errors.emplace_back(*e, "Identifier not found in current context.");
			return std::nullopt;
		}
		return ExpressionCheckResult(t->second, true);
	},
	[&](const std::unique_ptr<New>& e)->std::optional<ExpressionCheckResult>{
		const TypeInfo* t;
		call_with_error_log(errors, class_database.get(e->type), [&](auto tt){t = tt;});
		if(!t) {
			return std::nullopt;
		}
		std::vector<const TypeInfo*> args;
		for(const Expression& arg : e->args) {
			auto type = check_expr(arg);
			if(type) {
				args.push_back(type->type_info());
			}
		}
		if(t->constructible_with(args)) {
			return ExpressionCheckResult(t, false);
		} else {
			errors.emplace_back(*e, "Type " + t->full_name() + " not constructible with arguments " + types_to_string(args) + ".");
			return std::nullopt;
		}
	},
	[&](const std::unique_ptr<Negation>& e)->std::optional<ExpressionCheckResult>{
		auto t = check_expr(e->value);
		if(!t) {
			return std::nullopt;
		}
		const char* fun_name = e->boolean ? NEG_FUN_NAME : OPP_FUN_NAME;
		auto tt = t->type_info()->get_member(fun_name);
		if(!tt) {
			errors.emplace_back(*e, "Type " + t->type_info()->full_name() + " doesn't have member " + fun_name);
			return std::nullopt;
		}
		auto result = (**tt).call_with({});
		if(result) {
			return ExpressionCheckResult(*result, false);
		}
		errors.emplace_back(*e, "Type " + (**tt).full_name() + " cannot be called with ().");
		return std::nullopt;
	},
	[&](const std::unique_ptr<Cast>& e)->std::optional<ExpressionCheckResult>{
		auto et = check_expr(e->value);
		const TypeInfo* tt;
		call_with_error_log(errors, class_database.get(e->target_type), [&](const TypeInfo* t){tt = t;});
		if(!et || !tt) {
			return std::nullopt;
		}
		if(!et->type_info()->explicitly_convertible_to(tt)) {
			errors.emplace_back(*e, "Explicit cast of type " + et->type_info()->full_name() + " to " + tt->full_name() + " not possible.");
			return std::nullopt;
		}
		return ExpressionCheckResult(tt, false);
	},
	[&](const std::unique_ptr<Null>&)->std::optional<ExpressionCheckResult>{
		return ExpressionCheckResult(class_database.get_for_null(), false);
	},
	[&](const std::unique_ptr<This>&)->std::optional<ExpressionCheckResult>{
		return ExpressionCheckResult(variable_map.at(THIS_NAME), false);
	},
	[&](const std::unique_ptr<MemberAccess>& e)->std::optional<ExpressionCheckResult>{
		auto t = check_expr(e->object);
		if(!t) {
			return std::nullopt;
		}
		auto tt = t->type_info()->get_member(e->member.name);
		if(!tt) {
			errors.emplace_back(*e, "Type " + t->type_info()->full_name() + " doesn't have member " + e->member.name);
		}
		return ExpressionCheckResult(*tt, true);
	},
	[&](const std::unique_ptr<FunctionCall>& e)->std::optional<ExpressionCheckResult>{
		auto t = check_expr(e->function);
		std::vector<const TypeInfo*> args;
		for(const Expression& ee : e->arguments) {
			auto tt = check_expr(ee);
			if(tt) {
				args.push_back(tt->type_info());
			}
		}
		if(args.size() != e->arguments.size() || !t) {
			return std::nullopt;
		}
		auto result = t->type_info()->call_with(args);
		if(result) {
			return ExpressionCheckResult(*result, false);
		}
		errors.emplace_back(*e, "Type " + t->type_info()->full_name() + " cannot be called with (" + types_to_string(args) + ").");
		return std::nullopt;
	},
	[&](const std::unique_ptr<LazyFunctionCall>& e)->std::optional<ExpressionCheckResult>{
		auto t = check_expr(e->function);
		std::vector<const TypeInfo*> args;
		for(const auto&[expr, is_non_journal] : e->arguments) {
			auto tt = check_expr(expr);
			if(tt) {
				if(!tt->type_info()->check_nojournal_mark(is_non_journal)) {
					if(is_non_journal) {
						errors.emplace_back(expr, "Expression marked as non-journalable for a lazy call, but type supports journaling.");
					} else {
						errors.emplace_back(expr, "Type used in lazy call doesn't support journaling. If you know what you're doing use the ':' mark to silence this error.");
					}
				} else {
					args.push_back(std::move(tt->type_info()));
				}
			}
		}
		if(args.size() != e->arguments.size() || !t) {
			return std::nullopt;
		}
		auto result = t->type_info()->call_with(args);
		if(result) {
			return ExpressionCheckResult(*result, false);
		}
		errors.emplace_back(*e, "Type " + t->type_info()->full_name() + " cannot be called with (" + types_to_string(args) + ").");
		return std::nullopt;
	},
	[&](const std::unique_ptr<BinaryOperation>& e)->std::optional<ExpressionCheckResult>{
		auto left = check_expr(e->left);
		auto right = check_expr(e->right);
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
			auto f = left->type_info()->get_member(function_name);
			if(!f) {
				errors.emplace_back(*e, "Type " + left->type_info()->full_name() + " doesn't have member " + function_name);
				return std::nullopt;
			}
			auto result = (**f).call_with({right->type_info()});
			if(result) {
				return ExpressionCheckResult(*result, false);
			}
			errors.emplace_back(*e, "Type " + (**f).full_name() + " cannot be called with (" + right->type_info()->full_name() + ").");
			return std::nullopt;
		} else {
			if(!left->type_info()->comparable_with(right->type_info())) {
				errors.emplace_back(*e, "Types " + left->type_info()->full_name() + " and " + right->type_info()->full_name() + " cannot be compared using the '==' sign.");
			}
			return ExpressionCheckResult(class_database.get_for_int(), false);
		}
	}
	),e);
}

std::optional<std::vector<TypeError>> check_function_block(const TypeInfo* return_type, const std::optional<std::vector<const TypeInfo*>>& dual, std::map<std::string, const TypeInfo*>& variable_map, ClassDatabase& class_database, const Block& block){
	std::vector<TypeError> errors;
	std::vector<std::set<std::string>> variable_stack;
	auto add_variable_do = [&](const Identifier& name, const TypeInfo* type){
		if(variable_map.find(name.name) != variable_map.end()) {
			errors.emplace_back(name, "Variable redefinition.");
			return;
		}
		variable_stack[variable_stack.size() - 1].insert(name.name);
		variable_map.emplace(name.name, type);
	};

	auto add_variable = overload(
	add_variable_do,
	[&](const Identifier& name, const auto& type){
		call_with_error_log(errors, class_database.get(type), [&](auto t){
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

	auto get_type_convertibility_error = [&](const std::string& a, const std::string& b){
		return "Type " + a + " not convertible to " + b + ".";
	};

	auto check_expr = [&](const Expression& e) {
		return check_expression(errors, variable_map, class_database, e);
	};

	std::function<bool(const Statement&, bool)> check_statement;

	auto check_blk = [&](const Block& block, bool in_loop){
		bool returns = false;
		variable_stack.emplace_back();
		for(const auto& s : block.statements) {
			bool r = check_statement(s, in_loop);
			returns |= r;
		}
		remove_last_variable_block();
		return returns;
	};

	if(!errors.empty()) {
		return errors;
	}

	check_statement = [&](const Statement& statement, bool in_loop){
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
			return check_blk(*s, in_loop);
		},
		[&](const std::unique_ptr<Empty>&) {
			return false;
		},
		[&](const std::unique_ptr<StatementExpression>& s) {
			check_expr(s->expression);
			return false;
		},
		[&](const std::unique_ptr<For>& s) {
			variable_stack.emplace_back();
			check_statement(s->before, in_loop);
			auto condition_type = check_expr(s->condition);
			if(condition_type) {
				if(!condition_type->type_info()->implicitly_convertible_to(class_database.get_for_int())) {
					errors.emplace_back(s->condition, get_type_convertibility_error(condition_type->type_info()->full_name(), INT_TYPE_NAME));
				}
			}
			check_statement(s->after, in_loop);
			check_blk(*s->body, true);
			remove_last_variable_block();
			return false;
		},
		[&](const std::unique_ptr<VariableDeclaration>& s) {
			call_with_error_log(errors, class_database.get(s->type), [&](const TypeInfo* t){
				add_variable(s->name, t);
				if(s->value) {
					auto v = check_expr(*s->value);
					if(v && !v->type_info()->implicitly_convertible_to(t)) {
						errors.emplace_back(*s->value, get_type_convertibility_error(v->type_info()->full_name(), t->full_name()));
					}
				}
			});
			return false;
		},
		[&](const std::unique_ptr<Return>& s) {
			size_t direct = return_type != class_database.get_for_void();
			size_t dual_args = dual ? dual->size() : 0;
			if(direct + dual_args != s->values.size()) {
				errors.emplace_back(*s, "Wrong number of returned values, expected " + std::to_string(s->values.size()) + ", got " + std::to_string(dual_args + direct) + ".");
				return true;
			}
			if(direct) {
				auto t = check_expr(s->values[0]);
				if(t && !t->type_info()->implicitly_convertible_to(return_type)) {
					errors.emplace_back(s->values[0], get_type_convertibility_error(t->type_info()->full_name(), return_type->full_name()));
				}
			}
			for(size_t i = direct; i < dual_args; ++i) {
				auto t = check_expr(s->values[i]);
				if(t && !t->type_info()->implicitly_convertible_to((*dual)[i - direct])) {
					errors.emplace_back(s->values[i], get_type_convertibility_error(t->type_info()->full_name(), (*dual)[i - direct]->full_name()));
				}
			}
			return true;
		},
		[&](const std::unique_ptr<If>& s) {
			auto condition_type = check_expr(s->condition);
			if(condition_type) {
				if(!condition_type->type_info()->implicitly_convertible_to(class_database.get_for_int())) {
					errors.emplace_back(s->condition, get_type_convertibility_error(condition_type->type_info()->full_name(), INT_TYPE_NAME));
				}
			}
			bool returns = check_blk(*s->body, in_loop);
			if(s->body_else) {
				returns &= check_blk(**s->body_else, in_loop);
			} else {
				returns = false;
			}
			return returns;
		},
		[&](const std::unique_ptr<While>& s) {
			auto condition_type = check_expr(s->condition);
			if(condition_type) {
				if(!condition_type->type_info()->implicitly_convertible_to(class_database.get_for_int())) {
					errors.emplace_back(s->condition, get_type_convertibility_error(condition_type->type_info()->full_name(), INT_TYPE_NAME));
				}
			}
			check_blk(*s->body, true);
			return false;
		},
		[&](const std::unique_ptr<Assignment>& s) {
			auto var = check_expr(s->variable);
			auto val = check_expr(s->value);
			if(var && val) {
				if(!var->is_lvalue()) {
					errors.emplace_back(*s, "Expected lvalue as left side of assignment.");
				}
				if(!val->type_info()->implicitly_convertible_to(var->type_info())) {
					errors.emplace_back(*s, get_type_convertibility_error(var->type_info()->full_name(), val->type_info()->full_name()));
				}
			}
			return false;
		}
		), statement);
	};

	bool returns = check_blk(block, false);

	if(!returns && (return_type != class_database.get_for_void() || (dual && !dual->empty()))) {
		errors.emplace_back(block.end - 1, block.end, "Non-void function doesn't always return a value.");
	}

	if(errors.empty()) {
		return std::nullopt;
	}
	return errors;
}

std::optional<std::vector<TypeError>> RealFunctionInfo::check_body(std::map<std::string, const TypeInfo*> variable_map, ClassDatabase& class_database) const {
	std::vector<TypeError> errors;
	std::set<std::string> call_arguments;
	auto add_argument_do = [&](const Identifier& name, const TypeInfo* type){
		if(variable_map.find(name.name) != variable_map.end()) {
			errors.emplace_back(name, "Variable redefinition.");
			return;
		}
		variable_map.emplace(name.name, type);
		call_arguments.insert(name.name);
	};

	auto add_argument = overload(
	add_argument_do,
	[&](const Identifier& name, const auto& type){
		call_with_error_log(errors, class_database.get(type), [&](auto t){
			add_argument_do(name, t);
		});
	}
	);

	auto remove_arguments = [&](){
		for(const std::string& name : call_arguments) {
			variable_map.erase(name);
		}
		call_arguments.clear();
	};

	std::optional<std::vector<const TypeInfo*>> dual;

	if(dual_ast) {
		dual = std::vector<const TypeInfo*>{};
		for(const auto&[type, name] : (**dual_ast).first) {
			call_with_error_log(errors, class_database.get(type), [&](const TypeInfo* t){
				dual->push_back(t);
			});
		}
	}

	if(!errors.empty()) {
		return errors;
	}

	{
		if(body_ast) {
			for(size_t i = 0; i < arguments.size(); ++i) {
				add_argument((**body_ast).arguments[i].second, arguments[i]);
			}
			call_with_error_log(errors, check_function_block(return_type, dual, variable_map, class_database, **(**body_ast).body));
			remove_arguments();
		}
		if(dual_ast) {
			for(const auto& [type, name] : (**dual_ast).first) {
				add_argument(name, type);
			}
			call_with_error_log(errors, check_function_block(class_database.get_for_void(), std::nullopt, variable_map, class_database, *(**dual_ast).second));
			remove_arguments();
		}
	}

	if(errors.empty()) {
		return std::nullopt;
	}
	return errors;
}

std::variant<const TypeInfo*, std::vector<TypeError>> ClassDatabase::get(const VariableType& t) {
	return std::visit(overload(
	[&](const IntType&)->std::variant<const TypeInfo*, std::vector<TypeError>>{return get_for_int();},
	[&](const PointerType& t) {return std::visit([](auto v)->std::variant<const TypeInfo*, std::vector<TypeError>>{return v;}, get(t));}
	), t);
}

ClassDatabase::ClassDatabase() {
	types[NULL_TYPE_NAME][{}] = std::make_unique<NullTypeInfo>();
	types[INT_TYPE_NAME][{}] = std::make_unique<IntTypeInfo>();
	types[VOID_TYPE_NAME][{}] = std::make_unique<VoidTypeInfo>();
}

std::optional<const TypeInfo*> ClassDatabase::get(const std::string& name) const {
	{
		auto t = current_class_parameters.find(name);
		if(t != current_class_parameters.end()) {
			return t->second;
		}
	}
	auto t = types.find(name);
	if(t == types.end()) {
		return std::nullopt;
	}
	auto f = t->second.find({});
	if(f == t->second.end()) {
		return std::nullopt;
	} else {
		return f->second.get();
	}
}

std::optional<const TypeInfo*> RealFunctionInfo::call_with(const std::vector<const TypeInfo*>& args) const {
	if(args.size() != arguments.size()) {
		return std::nullopt;
	}
	for(size_t i = 0; i < args.size(); ++i) {
		if(!args[i]->implicitly_convertible_to(arguments[i])) {
			return std::nullopt;
		}
	}
	return return_type;
}

std::optional<std::vector<TypeError>> ClassDatabase::insert_pattern(const Class* p, std::shared_ptr<const std::string> filename) {
	if(patterns.find(p->name.name) != patterns.end()) {
		return std::vector<TypeError>{TypeError(p->name, "Class pattern redefinition.")};
	}
	std::vector<std::unique_ptr<const TypeInfo>> param_ti;
	std::vector<NonOwnedParameter> param_nop;
	{
		std::set<std::string> names;
		for(const Identifier& i : p->parameters) {
			if(names.count(i.name) || patterns.find(i.name) != patterns.end()) {
				return std::vector<TypeError>{TypeError(i, "Type parameter shadows another type related identifier.")};
			}
			names.insert(i.name);
		}
		if(!current_class_parameters.empty()) {
			throw std::runtime_error("Internal error.");
		}
		for(const Identifier& i : p->parameters) {
			auto t = std::make_unique<TemplateUnknownTypeInfo>(i.name);
			param_nop.push_back(add_parameter(i.name, t.get()));
			param_ti.push_back(std::move(t));
		}
	}
	auto context = std::make_shared<std::string>(p->parameters.empty() ? "During instatiation of the class " + p->name.name + "." : "During pattern type checking of class pattern " + p->name.name + ".");
	return std::visit(overload(
	[&](std::pair<std::unique_ptr<RealClassInfo>, std::vector<TypeError>> r)->std::optional<std::vector<TypeError>>{
		for(TypeError& oe : r.second) {
			oe.fill_filename_if_empty(filename);
			oe.add_context(context);
		}
		if(!r.second.empty()) {
			return std::move(r.second);
		}
		patterns.emplace(p->name.name, std::make_pair(p, filename));
		if(p->parameters.empty()) {
			if(types.find(p->name.name) != types.end()) {
				throw std::runtime_error("Internal error.");
			}
			types[p->name.name][{}] = std::move(r.first);
		}
		return std::nullopt;
	},
	[&](std::vector<TypeError> e)->std::optional<std::vector<TypeError>>{
		for(TypeError& oe : e) {
			oe.fill_filename_if_empty(filename);
			oe.add_context(context);
		}
		return e;
	}
	), RealClassInfo::make_class(*this, p));
}

ClassDatabase::NonOwnedParameter::NonOwnedParameter(ClassDatabase* cd, std::string name, const TypeInfo* type) : cd(cd) {
	if(cd->get(name)) {
		throw std::runtime_error("Internal error.");
	}
	cd->current_class_parameters.emplace(name, type);
}

ClassDatabase::NonOwnedParameter::~NonOwnedParameter() {
	if(cd) {
		auto f = cd->current_class_parameters.find(name);
		if(f == cd->current_class_parameters.end()) {
			std::terminate();
		}
		cd->current_class_parameters.erase(f);
	}
}

[[nodiscard]] ClassDatabase::NonOwnedParameter ClassDatabase::add_parameter(std::string name, const TypeInfo* type) {
	return NonOwnedParameter(this, move(name), type);
}

std::variant<const TypeInfo*, std::vector<TypeError>> ClassDatabase::get(const PointerType& t) {
	std::vector<TypeError> errors;
	std::vector<const TypeInfo*> params;
	for(const auto& p : t.parameters) {
		call_with_error_log(errors, get(p), [&](const TypeInfo* pp){
			if(temporary == pp) {
				errors.emplace_back(p, "Cannot use class inside itself as another type parameter.");
			} else {
				params.push_back(pp);
			}
		});
	}
	if(params.size() != t.parameters.size()) {
		return errors;
	}

	{
		if(t.parameters.empty()) {
			auto tt = current_class_parameters.find(t.name.name);
			if(tt != current_class_parameters.end()) {
				return tt->second;
			}
		}

		auto tt = types.find(t.name.name);
		if(tt != types.end()) {
			auto ttt = tt->second.find(params);
			if(ttt != tt->second.end()) {
				return ttt->second.get();
			}
		}
	}

	auto pattern = patterns.find(t.name.name);
	if(pattern == patterns.end()) {
		errors.emplace_back(t.name, "Cannot find pattern.");
		return errors;
	}

	std::map<std::string, const TypeInfo*> old_parameters = move(current_class_parameters);

	for(size_t i = 0; i < params.size(); ++i) {
		current_class_parameters.emplace(pattern->second.first->parameters[i].name, params[i]);
	}

	std::shared_ptr<std::string> context = std::make_shared<std::string>("During instatiation of the pattern " + t.name.name + " with parameters " + types_to_string(params) + ".");

	std::shared_ptr<const std::string> filename = pattern->second.second;

	return std::visit(overload(
		[&](std::pair<std::unique_ptr<RealClassInfo>, std::vector<TypeError>> r)->std::variant<const TypeInfo*, std::vector<TypeError>>{
			current_class_parameters = move(old_parameters);
			for(TypeError& oe : r.second) {
				oe.fill_filename_if_empty(filename);
				oe.add_context(context);
				optional_errors.push_back(std::move(oe));
			}
			RealClassInfo* ci = r.first.get();
			types[t.name.name][params] = std::move(r.first);
			return ci;
		},
		[&](std::vector<TypeError> e)->std::variant<const TypeInfo*, std::vector<TypeError>>{
			current_class_parameters = move(old_parameters);
			for(TypeError& oe : e) {
				oe.fill_filename_if_empty(filename);
				oe.add_context(context);
			}
			return e;
		}
		), RealClassInfo::make_class(*this, pattern->second.first));
}

#endif
