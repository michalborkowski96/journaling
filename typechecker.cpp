#include "typechecker.h"

using namespace ast;
using namespace expression;
using namespace statement;

namespace typechecker {

TypeError::TypeError(size_t begin, size_t end, std::string data) : begin(begin), end(end), data(move(data)) {}

TypeError::TypeError(const AstNode& node, std::string data) : begin(node.begin), end(node.end), data(move(data)) {}

TypeError::TypeError(const Expression& node, std::string data) : begin(std::visit([](const auto& p){return p->begin;}, node)), end(std::visit([](const auto& p){return p->end;}, node)), data(move(data)) {}

TypeError::TypeError(const Statement& node, std::string data) : begin(std::visit([](const auto& p){return p->begin;}, node)), end(std::visit([](const auto& p){return p->end;}, node)), data(move(data)) {}

TypeError::TypeError(const Type& node, std::string data) : begin(std::visit(overload([](const VariableType& p){return std::visit([](const auto& p){return p.begin;}, p);}, [](const auto& p){return p.begin;}), node)), end(std::visit(overload([](const VariableType& p){return std::visit([](const auto& p){return p.end;}, p);}, [](const auto& p){return p.end;}), node)), data(move(data)) {}

TypeError::TypeError(const VariableType& node, std::string data) : begin(std::visit([](const auto& p){return p.begin;}, node)), end(std::visit([](const auto& p){return p.end;}, node)), data(move(data)) {}

void TypeError::fill_filename_if_empty(std::shared_ptr<const std::string> fname) {
	if(!filename) {
		filename = fname;
	}
}

RealClassInfoView::RealClassInfoView(const RealClassInfo& rci) : parameters(rci.parameters), superclass(rci.superclass), variables(rci.variables), functions(rci.functions), constructors(rci.constructors), ast_data(rci.ast_data) {}

bool TypeError::operator==(const TypeError& o) const {
	if(begin != o.begin || end != o.end || data != o.data || (bool) filename != (bool) o.filename || context.size() != o.context.size()) {
		return false;
	}
	if(filename && (*filename) != (*o.filename)) {
		return false;
	}
	for(size_t i = 0; i < context.size(); ++i) {
		if((*context[i]) != (*o.context[i])) {
			return false;
		}
	}
	return true;
}

void TypeError::contract_errors(std::vector<TypeError>& errors) {
	std::vector<TypeError> new_errors;
	for(TypeError& e : errors) {
		bool found = false;
		for(size_t i = 0; i < new_errors.size(); ++i) {
			if(new_errors[i] == e) {
				found = true;
				break;
			}
		}
		if(!found) {
			new_errors.push_back(std::move(e));
		}
	}
	errors.swap(new_errors);
}

void TypeError::add_context(std::shared_ptr<const std::string> ctx){
	context.push_back(ctx);
}

void TypeError::print_errors(std::ostream& o, const std::vector<ParsedModule>& modules, const std::vector<TypeError>& errors, const char* color) {
	auto print_error = [&](const TypeError& e, const char* colour, const std::string& text) {
		o << "In file:\n" << **e.filename << "\n";
		o << position(text, e.begin, e.end) << "\n";
		for(auto c = e.context.crbegin(); c != e.context.crend(); ++c) {
			o << **c << "\n";
		}
		o << e.data << "\n";
		print_lines(text, e.begin, e.end, o, colour);
		o << "\n\n";
	};
	auto print_error_vec = [&](const std::vector<TypeError>& errors, const char* colour){
		for(const TypeError& e : errors) {
			if(!e.filename) {
				throw std::runtime_error("Internal error.");
			}
			const ParsedModule* pm = nullptr;
			for(const ParsedModule& m : modules) {
				if(m.path == **e.filename) {
					pm = &m;
					break;
				}
			}
			if(!pm) {
				throw std::runtime_error("Internal error.");
			}
			print_error(e, colour, pm->text);
		}
	};
	print_error_vec(errors, color);
}

bool TypeInfo::implicitly_convertible_from(const VoidTypeInfo*) const {
	return false;
}

bool TypeInfo::implicitly_convertible_from(const FunctionalTypeInfo*) const {
	return false;
}

bool TypeInfo::extendable() const {
	return false;
}
std::optional<const TypeInfo*> TypeInfo::get_superclass() const {
	return std::nullopt;
}
bool TypeInfo::constructible_with(const std::vector<const TypeInfo*>&) const {
	return false;
}
std::optional<const TypeInfo*> TypeInfo::call_with(const std::vector<const TypeInfo*>&) const {
	return std::nullopt;
}

bool TypeInfo::explicitly_convertible_to(const TypeInfo* o) const {
	return implicitly_convertible_to(o) || o->implicitly_convertible_to(this);
}

bool TypeInfo::comparable_with(const TypeInfo* o) const {
	return explicitly_convertible_to(o);
}


VariableMap::VariableMap(std::optional<const TypeInfo*> superclass) : superclass(superclass) {}
void VariableMap::add(std::string name, const TypeInfo* type) {
	if(vars.find(name) != vars.end()) {
		throw std::runtime_error("Internal error.");
	}
	vars.emplace(move(name), type);
}
bool VariableMap::try_add(std::string name, const TypeInfo* type) {
	if(vars.find(name) != vars.end()) {
		return false;
	}
	vars.emplace(move(name), type);
	return true;
}
void VariableMap::erase(const std::string& name) {
	if(vars.find(name) == vars.end()) {
		throw std::runtime_error("Internal error.");
	}
	vars.erase(name);
}


ClassDatabase::NonOwnedParameter::NonOwnedParameter(NonOwnedParameter&& o) : cd(o.cd), name(move(o.name)) {
	o.cd = nullptr;
}

std::vector<TypeError>& ClassDatabase::get_optional_errors() {
	return optional_errors;
}
const TypeInfo* ClassDatabase::get_for_null() const {
	return types.at(NULL_TYPE_NAME).at({}).get();
}
const TypeInfo* ClassDatabase::get_for_int() const {
	return types.at(INT_TYPE_NAME).at({}).get();
}
const TypeInfo* ClassDatabase::get_for_void() const {
	return types.at(VOID_TYPE_NAME).at({}).get();
}
std::optional<const TypeInfo*> ClassDatabase::get_for_string() const {
	return get(STRING_TYPE_NAME);
}
std::variant<const TypeInfo*, std::vector<TypeError>> ClassDatabase::get(const Identifier& t) {
	auto r = get(t.name);
	if(r) {
		return *r;
	}
	return std::vector<TypeError>{TypeError(t, "Class instance not found.")};
}
std::variant<const TypeInfo*, std::vector<TypeError>> ClassDatabase::get(const Type& t) {
	return std::visit(overload(
	[&](const VoidType&)->std::variant<const TypeInfo*, std::vector<TypeError>>{return get_for_void();},
	[&](const VariableType& tt) {return get(tt);}
	), t);
}

const TypeInfo* RealFunctionInfo::type_info() const {
	return type_info_data.get();
}

std::optional<FunctionKind> RealFunctionInfo::kind() const {
	return declaration_ast->kind;
}
bool RealFunctionInfo::fully_defined() const {
	if(kind() && (*kind()) == FunctionKind::DUAL) {
		return body_ast && dual;
	} else {
		return (bool) body_ast;
	}
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

class BuiltinIntFunctionInfo : public FunctionInfo {
protected:
	const TypeInfo* int_type;
	const char* name;
	bool arg;
	const std::unique_ptr<const TypeInfo> type_info_data;
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

bool VoidTypeInfo::implicitly_convertible_from(const RealClassInfo*) const {
	return false;
}
bool VoidTypeInfo::implicitly_convertible_from(const NullTypeInfo*) const {
	return false;
}
bool VoidTypeInfo::implicitly_convertible_from(const IntTypeInfo*) const {
	return false;
}
bool VoidTypeInfo::implicitly_convertible_from(const TemplateUnknownTypeInfo*) const {
	return false;
}
bool VoidTypeInfo::implicitly_convertible_from(const VoidTypeInfo*) const {
	return true;
}

bool VoidTypeInfo::check_nojournal_mark(bool mark) const {
	return !mark;
}
bool VoidTypeInfo::implicitly_convertible_to(const TypeInfo* o) const {
	return o->implicitly_convertible_from(this);
}
std::string VoidTypeInfo::full_name() const {
	return VOID_TYPE_NAME;
}
std::optional<const TypeInfo*> VoidTypeInfo::get_member(const std::string&) const {
	return std::nullopt;
}

bool IntTypeInfo::implicitly_convertible_from(const RealClassInfo*) const {
	return false;
}
bool IntTypeInfo::implicitly_convertible_from(const NullTypeInfo*) const {
	return true;
}
bool IntTypeInfo::implicitly_convertible_from(const IntTypeInfo*) const {
	return true;
}
bool IntTypeInfo::implicitly_convertible_from(const TemplateUnknownTypeInfo*) const {
	return true;
}
std::map<std::string, std::unique_ptr<BuiltinIntFunctionInfo>> IntTypeInfo::get_funs_map(const IntTypeInfo* int_type) {
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
	ins(EQUAL_FUN_NAME, true);
	ins(NOT_EQUAL_FUN_NAME, true);
	return funs;
}
IntTypeInfo::IntTypeInfo() : funs(get_funs_map(this)) {}
bool IntTypeInfo::check_nojournal_mark(bool mark) const {
	return !mark;
}
bool IntTypeInfo::implicitly_convertible_to(const TypeInfo* o) const {
	return o->implicitly_convertible_from(this);
}
std::string IntTypeInfo::full_name() const {
	return INT_TYPE_NAME;
}
std::optional<const TypeInfo*> IntTypeInfo::get_member(const std::string& name) const {
	auto it = funs.find(name);
	if(it == funs.end()) {
		return std::nullopt;
	}
	return it->second->type_info();
}

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
	virtual bool extendable() const override {
		return true;
	}
	virtual bool implicitly_convertible_to(const TypeInfo* o) const override {
		return o->implicitly_convertible_from(this);
	}
	virtual std::string full_name() const override {
		return name;
	}
	virtual std::optional<const TypeInfo*> call_with(const std::vector<const TypeInfo*>&) const override {
		return this;
	}
	virtual std::optional<const TypeInfo*> get_member(const std::string&) const override {
		return this;
	}
	TemplateUnknownTypeInfo() = delete;
	TemplateUnknownTypeInfo(std::string name) : name(std::move(name)) {}
	virtual ~TemplateUnknownTypeInfo() = default;
};

std::optional<ExpressionCheckResult> check_expression(std::vector<TypeError>& errors, const VariableMap& variable_map, ClassDatabase& class_database, const Expression& e);
std::optional<std::vector<TypeError>> check_function_block(const TypeInfo* return_type, const std::optional<std::vector<const TypeInfo*>>& dual, VariableMap& variable_map, ClassDatabase& class_database, const Block& block);

RealClassInfo::RealClassInfo(std::vector<const TypeInfo*> parameters, std::optional<const TypeInfo*> superclass, const ast::Class* ast_data) : parameters(move(parameters)), superclass(move(superclass)), ast_data(ast_data) {}

class PrivateContextFunctionInfo : public FunctionInfo {
	const FunctionInfo& fi;
public:
	PrivateContextFunctionInfo(const FunctionInfo& fi) : fi(fi) {}
	virtual const TypeInfo* type_info() const override {
		return fi.type_info();
	}
	virtual std::optional<ast::FunctionKind> kind() const override {
		return fi.kind();
	}
	virtual std::optional<const TypeInfo*> call_with(const std::vector<const TypeInfo*>& args) const override {
		return fi.call_with(args);
	}
	virtual std::string full_name() const override {
		return fi.full_name();
	}
	virtual ~PrivateContextFunctionInfo() = default;
};

bool RealClassInfo::implicitly_convertible_from(const RealClassInfo* o) const {
	if(this == o) {
		return true;
	}
	if(has_unknown_parameters() || o->has_unknown_parameters()) {
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
bool RealClassInfo::implicitly_convertible_from(const NullTypeInfo*) const {
	return true;
}
bool RealClassInfo::implicitly_convertible_from(const IntTypeInfo*) const {
	return false;
}
bool RealClassInfo::implicitly_convertible_from(const TemplateUnknownTypeInfo*) const {
	return true;
}

std::optional<const TypeInfo*> RealClassInfo::get_superclass() const {
	return superclass;
}

bool RealClassInfo::constructible_with(const std::vector<const TypeInfo*>& args) const {
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
const std::string& RealClassInfo::pattern_name() const {
	return ast_data->name.name;
}
bool RealClassInfo::has_nojournal_mark() const {
	return ast_data->nojournal;
}
std::optional<const TypeInfo*> RealClassInfo::get_member(const std::string& name) const {
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
	if(superclass) {
		return (**superclass).get_member(name);
	}
	return std::nullopt;
}
bool RealClassInfo::check_nojournal_mark(bool mark) const {
	return mark == ast_data->nojournal;
}
bool RealClassInfo::extendable() const {
	return !ast_data->public_variables;
}
std::variant<std::pair<std::unique_ptr<RealClassInfo>, std::vector<TypeError>>, std::vector<TypeError>> RealClassInfo::make_class(ClassDatabase& class_database, const Class* ast) {
	std::vector<TypeError> errors;
	if(ast->parameters.empty() && !ast->optional_functions.empty()) {
		errors.emplace_back(ast->name, "Class without parameters must not have optional functions.");
	}
	std::unique_ptr<RealClassInfo> type_info;
	{
		std::optional<const TypeInfo*> superclass;
		if(ast->superclass) {
			if(ast->public_variables) {
				errors.emplace_back(ast->name, "Struct-like class must not extend another class.");
			}
			call_with_error_log(errors, class_database.get(*ast->superclass), [&](const TypeInfo* type){
				if(!type->extendable()) {
					errors.emplace_back(*(ast->superclass), "Cannot extend struct-like classes.");
				} else if(!type->check_nojournal_mark(ast->nojournal)) {
					if(ast->nojournal) {
						errors.emplace_back(*(ast->superclass), "Extending journalable class with a nojournal one.");
					} else {
						errors.emplace_back(*(ast->superclass), "Extending nojournal class with a journalable one.");
					}
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

		if(ast->name.name == "JournalTag" && parameters.size() == 1) {
			const RealClassInfo* rci = dynamic_cast<const RealClassInfo*>(parameters.front());
			if(rci && rci->has_nojournal_mark()) {
				errors.emplace_back(ast->parameters.front(), "JournalTag can be instantiated only with journalable types and 'int'.");
			}
		}

		if(!errors.empty()) {
			return errors;
		}

		type_info = std::unique_ptr<RealClassInfo>(new RealClassInfo(move(parameters), superclass, ast));
	}

	auto temp = class_database.add_parameter(ast->name.name, type_info.get());

	{
		auto add_variable = [&](const ClassVariable& node, std::string name, const TypeInfo* type){
			if(std::visit(overload([](const ClassVariableInteger&){return false;}, [](const ClassVariablePointer& n){return !n.strong;}), node) && dynamic_cast<const IntTypeInfo*>(type)) {
				std::visit([&](const auto& n){errors.emplace_back(n, "Weak pointers cannot be of builtin type 'int'.");}, node);
			} else if(type_info->variables.find(name) != type_info->variables.end()) {
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
			errors.emplace_back(f.name, "Function overloading not supported.");
		}
		if(!ast->abstract && !fi.fully_defined()) {
			errors.emplace_back(f.name, "Function not fully defined in non-abstract class.");
		}
		if(ast->nojournal && fi.kind()) {
			errors.emplace_back(f.name, "Nojournal class must not specify function kind.");
		}
		if(!ast->nojournal && !fi.kind()) {
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

	std::map<std::string, std::unique_ptr<PrivateContextFunctionInfo>> private_context_function_info;
	std::map<std::string, std::unique_ptr<FunctionalTypeInfo>> private_context_function_type_info;

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
			call_with_error_log(errors, RealFunctionInfo::make_function_declaration(overridden, class_database, &f, ast->nojournal), [&](std::unique_ptr<RealFunctionInfo>&& fi){
				call_with_error_log(errors, verify_function_declaration(false, f, *fi), [&](){
					private_context_function_info.emplace(f.name.name, std::make_unique<PrivateContextFunctionInfo>(*fi));
					private_context_function_type_info.emplace(f.name.name, std::make_unique<FunctionalTypeInfo>(private_context_function_info.at(f.name.name).get()));
					type_info->functions.emplace(f.name.name, std::move(fi));
				});
			});
		}
		if(!errors.empty()) {
			return errors;
		}
	}

	VariableMap base_variables(type_info->superclass);

	{
		const TypeInfo* bs = type_info.get();
		base_variables.add(THIS_NAME, bs);
	}

	for(const auto&[name, t] : type_info->variables) {
		base_variables.add(name, t);
	}

	for(const auto&[name, f] : private_context_function_type_info) {
		base_variables.add(name, f.get());
	}

	for(const auto&[name, f] : type_info->functions) {
		call_with_error_log(errors, f->check_body(base_variables, class_database));
	}

	auto check_constructor_body = [&errors, &base_variables, &class_database, &superclass = type_info->superclass](const Constructor& ast, const std::vector<const TypeInfo*>& args){
		VariableMap variable_map = base_variables;
		VariableMap superclass_call_variable_map(std::nullopt);
		auto add_variable = [&](const Identifier& name, const TypeInfo* type){
			if(!variable_map.try_add(name.name, type)) {
				errors.emplace_back(name, "Variable redefinition.");
				return;
			}
			superclass_call_variable_map.add(name.name, type);
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
						errors.emplace_back(ast.begin, ast.body->begin, "Superclass cannot be constructed with arguments (" + types_to_string(superclass_call_args) + ").");
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
	if(ast->destructor) {
		call_with_error_log(errors, check_function_block(class_database.get_for_void(), std::nullopt, base_variables, class_database, *ast->destructor->body));
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
			call_with_error_log(optional_errors, RealFunctionInfo::make_function_declaration(overridden, class_database, &f, ast->nojournal), [&](std::unique_ptr<RealFunctionInfo>&& fi){
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
			private_context_function_info.emplace(name, std::make_unique<PrivateContextFunctionInfo>(*f));
			private_context_function_type_info.emplace(name, std::make_unique<FunctionalTypeInfo>(private_context_function_info.at(name).get()));
			base_variables.add(name, private_context_function_type_info.at(name).get());
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

	if(!ast->abstract && type_info->superclass) {
		const RealClassInfo* superclass = dynamic_cast<const RealClassInfo*>(*type_info->superclass);
		while(superclass && superclass->ast_data->abstract) {
			for(const auto&[name, data] : superclass->functions) {
				if(!data->fully_defined() && type_info->functions.find(name) == type_info->functions.end()) {
					errors.emplace_back(ast->name, "Function " + name + " inherited from " + superclass->full_name() + " not fully defined in a non-abstract class.");
				}
			}
			if(superclass->superclass) {
				superclass = dynamic_cast<const RealClassInfo*>(*superclass->superclass);
			} else {
				break;
			}
		}
	}

	if(!errors.empty()) {
		return errors;
	}
	return std::make_pair(move(type_info), move(optional_errors));
}
std::optional<const RealFunctionInfo*> RealClassInfo::get_function(const std::string& name) const {
	auto f = functions.find(name);
	if(f == functions.end()) {
		return std::nullopt;
	}
	return f->second.get();
}
bool RealClassInfo::implicitly_convertible_to(const TypeInfo* o) const {
	return o->implicitly_convertible_from(this);
}
std::string RealClassInfo::full_name() const {
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

std::string types_to_string(const std::vector<const TypeInfo*>& v) {
	std::string arg_types;
	if(v.size()) {
		arg_types = v[0]->full_name();
	}
	for(size_t i = 1; i < v.size(); ++i) {
		arg_types += ", ";
		arg_types += v[i]->full_name();
	}
	return arg_types;
}

bool RealClassInfo::comes_from_same_pattern(const RealClassInfo& o) const {
	return ast_data == o.ast_data;
}

std::variant<std::unique_ptr<RealFunctionInfo>, std::vector<TypeError>> RealFunctionInfo::make_function_declaration(std::optional<const RealFunctionInfo*> overridden, ClassDatabase& class_database, const Function* ast, bool comes_from_nojournal) {
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
		body = (**overridden).body_ast;
	}
	std::optional<std::pair<const std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<statement::Block>>*, std::vector<const TypeInfo*>>> dual;
	if(ast->dual) {
		std::vector<const TypeInfo*> dual_args;
		for(const auto&[type, name] : ast->dual->first) {
			call_with_error_log(errors, class_database.get(type), [&](const TypeInfo* t){
				dual_args.push_back(t);
			});
		}
		dual = std::make_pair(&*ast->dual, std::move(dual_args));
	} else if(overridden) {
		dual = (**overridden).dual;
	}

	if(!errors.empty()) {
		return errors;
	}

	if(overridden) {
		const RealFunctionInfo& fi = **overridden;
		auto same_types = [](const TypeInfo* a, const TypeInfo* b){
			if(dynamic_cast<const TemplateUnknownTypeInfo*>(a) || dynamic_cast<const TemplateUnknownTypeInfo*>(b)) {
				return true;
			}
			auto aa = dynamic_cast<const RealClassInfo*>(a);
			auto ab = dynamic_cast<const RealClassInfo*>(b);
			if(aa && ab) {
				if(!aa->has_unknown_parameters() && !ab->has_unknown_parameters()) {
					return a == b;
				}
				return aa->comes_from_same_pattern(*ab);
			}
			return a == b;
		};
		{
			bool same_arg_count = fi.arguments.size() == args.size();
			bool same_arg_types = true;
			if(same_arg_count) {
				for(size_t i = 0; i < fi.arguments.size(); ++i) {
					if(!same_types(fi.arguments[i], args[i])) {
						same_arg_types = false;
					}
				}
			}
			if(!same_arg_count || !same_arg_types) {
				errors.emplace_back(ast->name, "Function overrides another function, but has different arguments, (" + types_to_string(args) + " instead of " + types_to_string(fi.arguments) + ").");
			}
		}
		if(!same_types(rt, fi.return_type)) {
			errors.emplace_back(ast->name, "Function overrides another function, but has different return type, (" + rt->full_name() + " instead of " + fi.return_type->full_name() + ").");
		}

		if(ast->kind != fi.kind() && ((ast->kind == FunctionKind::NOEFFECT) || (fi.kind() == FunctionKind::NOEFFECT))) {
			errors.emplace_back(ast->name, "Noefffect can only be overridden by noefffect and vice versa.");
		}
	}

	if(ast->kind == FunctionKind::DUAL && !dual && body) {
		errors.emplace_back(ast->name, "Dual function has primary definition, but no dual definition.");
	}

	if(ast->kind != FunctionKind::DUAL) {
		dual = {};
	}

	if(!errors.empty()) {
		return errors;
	} else {
		return std::unique_ptr<RealFunctionInfo>(new RealFunctionInfo(ast->name.name, std::move(rt), move(args), ast, body, dual, !comes_from_nojournal));
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

RealFunctionInfo::RealFunctionInfo(std::string name, const TypeInfo* return_type, std::vector<const TypeInfo*> arguments, const Function* declaration_ast, std::optional<const Function*> body_ast, std::optional<std::pair<const std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<statement::Block>>*, std::vector<const TypeInfo*>>> dual, bool nojournal_marks) : name(move(name)), return_type(std::move(return_type)), type_info_data(std::make_unique<FunctionalTypeInfo>(this)), arguments(move(arguments)), declaration_ast(declaration_ast), body_ast(body_ast), dual(std::move(dual)), nojournal_marks(nojournal_marks) {}

std::optional<ExpressionCheckResult> check_expression(std::vector<TypeError>& errors, const VariableMap& variable_map, ClassDatabase& class_database, const Expression& e) {
	auto check_expr = [&](const Expression& e) {
		return check_expression(errors, variable_map, class_database, e);
	};
	return std::visit(overload(
	[&](const std::unique_ptr<StringLiteral>& e)->std::optional<ExpressionCheckResult>{
		auto t = class_database.get_for_string();
		if(t) {
			auto construct_result = (**t).constructible_with({class_database.get_for_int(), class_database.get_for_int()});
			if(!construct_result) {
				errors.emplace_back(*e, "String class must have (int, int) contructor to use string literals.");
				return std::nullopt;
			} else {
				return ExpressionCheckResult(*t, false);
			}
		} else {
			errors.emplace_back(*e, "String library required to use string literals.");
			return std::nullopt;
		}
	},
	[&](const std::unique_ptr<IntegerLiteral>&)->std::optional<ExpressionCheckResult>{
		return ExpressionCheckResult(class_database.get_for_int(), false);
	},
	[&](const std::unique_ptr<Lambda>& l)->std::optional<ExpressionCheckResult>{
		VariableMap inner(std::nullopt);
		for(const auto&[var, capture_type] : l->capture) {
			if(variable_map.has(var.name)) {
				const TypeInfo* type = variable_map.at(var.name);
				const RealClassInfo* rci = dynamic_cast<const RealClassInfo*>(type);
				if(capture_type == Lambda::CaptureType::JOURNAL && rci && rci->has_nojournal_mark()) {
					errors.emplace_back(var, "Journal capture requested for a type that doesn't support it.");
				}
				if(!type->implicitly_convertible_to(type)) {
					errors.emplace_back(var, "Type " + type->full_name() + " cannot be captured.");
				} else if(!inner.try_add(var.name, type)) {
					errors.emplace_back(var, "Capture redefinition.");
				}
			} else {
				errors.emplace_back(var, "Identifier not found in current context.");
			}
		}
		std::vector<const TypeInfo*> arguments;
		for(const auto&[type, name] : l->arguments) {
			std::visit(overload([&, &name = name](const TypeInfo* type_info){
				if(!inner.try_add(name.name, type_info)) {
					errors.emplace_back(name, "Variable redefinition.");
				} else {
					arguments.push_back(type_info);
				}
			}, [&](std::vector<TypeError>&& e) {
				for(TypeError& ee : e) {
					errors.push_back(std::move(ee));
				}
			}), class_database.get(type));
		}
		const TypeInfo* return_type = nullptr;
		std::visit(overload([&](const TypeInfo* type_info){
			return_type = type_info;
		}, [&](std::vector<TypeError>&& e) {
			for(TypeError& ee : e) {
				errors.push_back(std::move(ee));
			}
		}), class_database.get(l->return_type));
		if(return_type) {
			std::optional<std::vector<TypeError>> r = check_function_block(return_type, std::nullopt, inner, class_database, *l->body);
			if(r) {
				for(TypeError& ee : *r) {
					errors.push_back(std::move(ee));
				}
			}
		}
		size_t args_begin = l->body->begin;
		size_t args_end = args_begin;
		if(!l->arguments.empty()) {
			args_begin = std::visit([](const auto& r){return r.begin;}, l->arguments.front().first);
			args_end = l->arguments.back().second.end;
		}
		if(return_type && arguments.size() == l->arguments.size()) {
			const TypeInfo* functional_type = nullptr;
			std::visit(overload([&](const TypeInfo* type_info){
				functional_type = type_info;
			}, [&](std::vector<TypeError>&& e) {
				for(TypeError& ee : e) {
					errors.push_back(std::move(ee));
				}
			}), class_database.get_functional(args_begin, args_end, return_type, arguments));
			if(functional_type) {
					return ExpressionCheckResult(functional_type, false);
			} else {
				return std::nullopt;
			}
		} else {
			return std::nullopt;
		}
	},
	[&](const std::unique_ptr<Identifier>& e)->std::optional<ExpressionCheckResult>{
		if(!variable_map.has(e->name)) {
			errors.emplace_back(*e, "Identifier not found in current context.");
			return std::nullopt;
		}
		return ExpressionCheckResult(variable_map.at(e->name), true);
	},
	[&](const std::unique_ptr<New>& e)->std::optional<ExpressionCheckResult>{
		const TypeInfo* t = nullptr;
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
		if(!t->constructible_with(args)) {
			errors.emplace_back(*e, "Type " + t->full_name() + " not constructible with arguments (" + types_to_string(args) + ").");
		}
		return ExpressionCheckResult(t, false);
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
			return std::nullopt;
		}
		return ExpressionCheckResult(*tt, true);
	},
	[&](const std::unique_ptr<FunctionCall>& e)->std::optional<ExpressionCheckResult>{
		auto t = check_expr(e->function);
		std::vector<const TypeInfo*> args;
		if(!t) {
			return std::nullopt;
		}
		for(const auto& expr : e->arguments) {
			auto tt = check_expr(expr);
			if(tt) {
				args.push_back(std::move(tt->type_info()));
			}
		}
		if(args.size() != e->arguments.size()) {
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
				return std::nullopt;
			}
			return ExpressionCheckResult(class_database.get_for_int(), false);
		}
	}
	),e);
}

std::optional<std::vector<TypeError>> check_function_block(const TypeInfo* return_type, const std::optional<std::vector<const TypeInfo*>>& dual, VariableMap& variable_map, ClassDatabase& class_database, const Block& block){
	std::vector<TypeError> errors;
	std::vector<std::set<std::string>> variable_stack;
	auto add_variable_do = [&](const Identifier& name, const TypeInfo* type){
		if(!variable_map.try_add(name.name, type)) {
			errors.emplace_back(name, "Variable redefinition.");
			return;
		}
		variable_stack.back().insert(name.name);
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
		for(const std::string& name : variable_stack.back()) {
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
				if(s->value) {
					auto v = check_expr(*s->value);
					if(v && !v->type_info()->implicitly_convertible_to(t)) {
						errors.emplace_back(*s->value, get_type_convertibility_error(v->type_info()->full_name(), t->full_name()));
					}
				}
				add_variable(s->name, t);
			});
			return false;
		},
		[&](const std::unique_ptr<VariableAuto>& s) {
			auto v = check_expr(s->value);
			if(v) {
				if(dynamic_cast<const VoidTypeInfo*>(v->type_info())) {
					errors.emplace_back(s->value, "auto type must not resolve to void");
				}
				add_variable(s->name, v->type_info());
			}
			return false;
		},
		[&](const std::unique_ptr<Return>& s) {
			size_t direct = return_type != class_database.get_for_void();
			size_t dual_args = dual ? dual->size() : 0;
			if(direct + dual_args != s->values.size()) {
				errors.emplace_back(*s, "Wrong number of returned values, expected " + std::to_string(dual_args + direct) + ", got " + std::to_string(s->values.size()) + ".");
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
					errors.emplace_back(*s, get_type_convertibility_error(val->type_info()->full_name(), var->type_info()->full_name()));
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

std::optional<std::vector<TypeError>> RealFunctionInfo::check_body(VariableMap& variable_map, ClassDatabase& class_database) const {
	std::shared_ptr<std::string> context = std::make_shared<std::string>("In function " + name);
	std::vector<TypeError> errors;
	std::set<std::string> call_arguments;
	auto add_context = [&](){
		for(TypeError& e : errors) {
			e.add_context(context);
		}
	};
	auto add_argument_do = [&](const Identifier& name, const TypeInfo* type){
		if(!variable_map.try_add(name.name, type)) {
			errors.emplace_back(name, "Variable redefinition.");
			add_context();
			return;
		}
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

	if(!errors.empty()) {
		add_context();
		return errors;
	}

	{
		if(body_ast == declaration_ast) {
			for(size_t i = 0; i < arguments.size(); ++i) {
				add_argument(declaration_ast->arguments[i].second, arguments[i]);
			}
			call_with_error_log(errors, check_function_block(return_type, dual ? std::optional<std::vector<const TypeInfo*>>(dual->second) : std::optional<std::vector<const TypeInfo*>>(std::nullopt), variable_map, class_database, **declaration_ast->body));
			for(const std::string& name : call_arguments) {
				variable_map.erase(name);
			}
			call_arguments.clear();
		}
		if(declaration_ast->dual) {
			for(const auto& [type, name] : dual->first->first) {
				add_argument(name, type);
			}
			call_with_error_log(errors, check_function_block(class_database.get_for_void(), std::nullopt, variable_map, class_database, *dual->first->second));
			remove_arguments();
		}
	}

	if(errors.empty()) {
		return std::nullopt;
	}
	add_context();
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
	auto context = std::make_shared<std::string>(p->parameters.empty() ? "During instatiation of the class " + p->name.name + "." : "During pattern type checking of class pattern " + p->name.name + ".");
	auto make_error = [&](const AstNode& n, std::string s){
		std::vector<TypeError> err{TypeError(n, std::move(s))};
		err[0].fill_filename_if_empty(filename);
		err[0].add_context(context);
		return err;
	};
	if(patterns.find(p->name.name) != patterns.end()) {
		return make_error(p->name, "Class pattern redefinition.");
	}
	std::vector<std::unique_ptr<const TypeInfo>> param_ti;
	std::vector<NonOwnedParameter> param_nop;
	{
		std::set<std::string> names;
		for(const Identifier& i : p->parameters) {
			if(names.count(i.name) || patterns.find(i.name) != patterns.end()) {
				return make_error(i, "Type parameter shadows another type related identifier.");
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
	size_t optional_error_count = optional_errors.size();
	return std::visit(overload(
	[&](std::pair<std::unique_ptr<RealClassInfo>, std::vector<TypeError>> r)->std::optional<std::vector<TypeError>>{
		for(TypeError& oe : r.second) {
			oe.fill_filename_if_empty(filename);
			oe.add_context(context);
		}
		temporary_types_with_unknown_parameters.clear();
		if(!r.second.empty()) {
			return std::move(r.second);
		} else {
			for(TypeError& oe : r.second) {
				oe.fill_filename_if_empty(filename);
				optional_errors.push_back(std::move(oe));
			}
			for(size_t i = optional_error_count; i < optional_errors.size() - r.second.size(); ++i) {
				optional_errors[i].add_context(context);
			}
		}
		patterns.emplace(p->name.name, std::make_pair(p, filename));
		if(p->parameters.empty()) {
			if(types.find(p->name.name) != types.end()) {
				throw std::runtime_error("Internal error.");
			}
			classes_order.emplace_back(r.first.get());
			types[p->name.name][{}] = std::move(r.first);
		}
		return std::nullopt;
	},
	[&](std::vector<TypeError> e)->std::optional<std::vector<TypeError>>{
		for(TypeError& oe : e) {
			oe.fill_filename_if_empty(filename);
			oe.add_context(context);
		}
		temporary_types_with_unknown_parameters.clear();
		return e;
	}
	), RealClassInfo::make_class(*this, p));
}

bool RealClassInfo::has_unknown_parameters() const {
	for(const TypeInfo* p : parameters) {
		if(dynamic_cast<const TemplateUnknownTypeInfo*>(p)) {
			return true;
		}
		if(const RealClassInfo* pp = dynamic_cast<const RealClassInfo*>(p)) {
			if(pp->has_unknown_parameters()) {
				return true;
			}
		}
	}
	return false;
}

ClassDatabase::NonOwnedParameter::NonOwnedParameter(ClassDatabase* cd, std::string name, const TypeInfo* type) : cd(cd), name(move(name)) {
	if(cd->get(this->name)) {
		throw std::runtime_error("Internal error.");
	}
	cd->current_class_parameters.emplace(this->name, type);
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

std::variant<const TypeInfo*, std::vector<TypeError>> ClassDatabase::get_functional(size_t args_begin, size_t args_end, const TypeInfo* return_type, const std::vector<const TypeInfo*>& arguments) {
	std::string pattern_name = "Function" + std::to_string(arguments.size());
	std::vector<const TypeInfo*> params{return_type};
	for(const auto ti : arguments) {
		params.push_back(ti);
	}
	{
		auto tt = types.find(pattern_name);
		if(tt != types.end()) {
			auto ttt = tt->second.find(params);
			if(ttt != tt->second.end()) {
				return ttt->second.get();
			}
		}
	}
	auto pattern = patterns.find(pattern_name);
	if(pattern == patterns.end()) {
		return std::vector<TypeError>{TypeError(args_begin, args_end, "Pattern '" + pattern_name + "' not found in current context. Please use the 'function' library and make sure it has declarations with enough arguments.")};
	}
	if(pattern->second.first->parameters.size() != params.size()) {
		return std::vector<TypeError>{TypeError(args_begin, args_end, "Pattern '" + pattern_name + "' takes " + std::to_string(pattern->second.first->parameters.size()) + " arguments, not " + std::to_string(params.size()) + ". This is a bug in the standard library. Did you mess with it?")};
	}

	std::map<std::string, const TypeInfo*> old_parameters = move(current_class_parameters);

	for(size_t i = 0; i < params.size(); ++i) {
		current_class_parameters.emplace(pattern->second.first->parameters[i].name, params[i]);
	}

	std::shared_ptr<const std::string> context = std::make_shared<std::string>("During instatiation of the pattern " + pattern_name + " with parameters <" + types_to_string(params) + ">.");

	std::shared_ptr<const std::string> filename = pattern->second.second;

	size_t optional_error_count = optional_errors.size();

	return std::visit(overload(
		[&](std::pair<std::unique_ptr<RealClassInfo>, std::vector<TypeError>> r)->std::variant<const TypeInfo*, std::vector<TypeError>>{
			current_class_parameters = move(old_parameters);
			for(TypeError& oe : r.second) {
				oe.fill_filename_if_empty(filename);
				optional_errors.push_back(std::move(oe));
			}
			for(size_t i = optional_error_count; i < optional_errors.size(); ++i) {
				optional_errors[i].add_context(context);
			}
			RealClassInfo* ci = r.first.get();
			if(!ci->has_unknown_parameters()) {
				classes_order.push_back(ci);
				types[pattern_name][params] = std::move(r.first);
			} else {
				temporary_types_with_unknown_parameters.push_back(std::move(r.first));
			}
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

std::variant<const TypeInfo*, std::vector<TypeError>> ClassDatabase::get(const PointerType& t) {
	std::vector<TypeError> errors;
	std::vector<const TypeInfo*> params;
	for(const auto& p : t.parameters) {
		call_with_error_log(errors, get(p), [&](const TypeInfo* pp){
			/*const RealClassInfo* ppp = dynamic_cast<const RealClassInfo*>(pp);
			if(ppp && current_class_parameters.find(ppp->pattern_name()) != current_class_parameters.end()) {
				errors.emplace_back(p, "Cannot use class inside itself as another type parameter.");
			} else */{
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
	if(pattern->second.first->parameters.size() != params.size()) {
		errors.emplace_back(t, "Wrong number of parameters.");
		return errors;
	}

	std::map<std::string, const TypeInfo*> old_parameters = move(current_class_parameters);

	for(size_t i = 0; i < params.size(); ++i) {
		current_class_parameters.emplace(pattern->second.first->parameters[i].name, params[i]);
	}

	std::shared_ptr<const std::string> context = std::make_shared<std::string>("During instatiation of the pattern " + t.name.name + " with parameters <" + types_to_string(params) + ">.");

	std::shared_ptr<const std::string> filename = pattern->second.second;

	size_t optional_error_count = optional_errors.size();

	return std::visit(overload(
		[&](std::pair<std::unique_ptr<RealClassInfo>, std::vector<TypeError>> r)->std::variant<const TypeInfo*, std::vector<TypeError>>{
			current_class_parameters = move(old_parameters);
			for(TypeError& oe : r.second) {
				oe.fill_filename_if_empty(filename);
				optional_errors.push_back(std::move(oe));
			}
			for(size_t i = optional_error_count; i < optional_errors.size(); ++i) {
				optional_errors[i].add_context(context);
			}
			RealClassInfo* ci = r.first.get();
			if(!ci->has_unknown_parameters()) {
				classes_order.push_back(ci);
				types[t.name.name][params] = std::move(r.first);
			} else {
				temporary_types_with_unknown_parameters.push_back(std::move(r.first));
			}
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

const TypeInfo* VariableMap::at(const std::string& name) const {
	if(vars.find(name) != vars.end()) {
		return vars.at(name);
	}
	return *(*superclass)->get_member(name);
}

bool VariableMap::has(const std::string& name) const {
	if(vars.find(name) != vars.end()) {
		return true;
	}
	if(superclass) {
		return (bool) (*superclass)->get_member(name);
	}
	return false;
}

const std::vector<const RealClassInfo*>& ClassDatabase::get_ordered() const {
	return classes_order;
}

std::pair<std::pair<std::vector<TypeError>, std::vector<TypeError>>, ClassDatabase> typecheck(const std::vector<ParsedModule>& modules) {
	std::vector<TypeError> errors;
	ClassDatabase class_database;
	for(const ParsedModule& m : modules) {
		auto path = std::make_shared<std::string>(m.path);
		for(const Class& c : m.module.classes) {
			call_with_error_log(errors, class_database.insert_pattern(&c, path));
		}
	}
	return std::make_pair(std::make_pair(std::move(errors), class_database.get_optional_errors()), std::move(class_database));
}

RealFunctionInfoView::RealFunctionInfoView(const RealFunctionInfo& f) : name(f.name), return_type(f.return_type), type_info_data(f.type_info_data.get()), arguments(f.arguments), declaration_ast(f.declaration_ast), body_ast(f.body_ast), dual(f.dual) {}

}
