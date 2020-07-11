#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include "ast.h"
#include "parser.h"
#include "util.h"

#include <vector>
#include <optional>
#include <functional>
#include <map>
#include <set>
#include <filesystem>

namespace typechecker {

struct TypeError {
	size_t begin;
	size_t end;
	std::string data;
	std::optional<std::shared_ptr<const std::string>> filename;
	std::vector<std::shared_ptr<const std::string>> context;
	TypeError() = delete;
	TypeError(size_t, size_t, std::string);
	TypeError(const ast::AstNode&, std::string);
	TypeError(const ast::expression::Expression&, std::string);
	TypeError(const ast::statement::Statement&, std::string);
	TypeError(const ast::Type&, std::string);
	TypeError(const ast::VariableType&, std::string);
	void fill_filename_if_empty(std::shared_ptr<const std::string> fname);
	void add_context(std::shared_ptr<const std::string> ctx);
	static void print_errors(std::ostream& o, const std::vector<ParsedModule>& modules, const std::vector<TypeError>& errors, const char* color);
	static void contract_errors(std::vector<TypeError>& errors);
	bool operator==(const TypeError&) const;
};

class RealClassInfo;
class NullTypeInfo;
class IntTypeInfo;
class TemplateUnknownTypeInfo;
class VoidTypeInfo;
class FunctionalTypeInfo;
class FunctionInfo;
class BuiltinIntFunctionInfo;
class PrivateContextFunctionInfo;

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
	virtual bool implicitly_convertible_from(const VoidTypeInfo*) const;
	bool implicitly_convertible_from(const FunctionalTypeInfo*) const;
	TypeInfo() = default;
	TypeInfo(const TypeInfo&) = delete;
	TypeInfo(TypeInfo&&) = delete;
public:
	virtual bool extendable() const;
	virtual std::optional<const TypeInfo*> get_superclass() const;
	virtual bool constructible_with(const std::vector<const TypeInfo*>&) const;
	virtual std::optional<const TypeInfo*> call_with(const std::vector<const TypeInfo*>&) const;
	virtual bool check_nojournal_mark(bool mark) const = 0;
	virtual bool implicitly_convertible_to(const TypeInfo* o) const = 0;
	virtual std::string full_name() const = 0;
	virtual std::optional<const TypeInfo*> get_member(const std::string& name) const = 0;
	virtual ~TypeInfo() = default;
	bool explicitly_convertible_to(const TypeInfo* o) const;
	bool comparable_with(const TypeInfo* o) const;
	friend void print(std::ostream&, const std::vector<const RealClassInfo*>&, const std::filesystem::path& path_to_cpp_library);
};

class FunctionInfo {
protected:
	FunctionInfo() = default;
public:
	FunctionInfo(const FunctionInfo&) = delete;
	FunctionInfo(FunctionInfo&&) = delete;
	virtual const TypeInfo* type_info() const = 0;
	virtual std::optional<ast::FunctionKind> kind() const = 0;
	virtual std::optional<const TypeInfo*> call_with(const std::vector<const TypeInfo*>& args) const = 0;
	virtual std::string full_name() const = 0;
	virtual ~FunctionInfo() = default;
};

class VariableMap {
	std::map<std::string, const TypeInfo*> vars;
	std::optional<const TypeInfo*> superclass;
public:
	VariableMap(std::optional<const TypeInfo*> superclass);
	VariableMap() = delete;
	void add(std::string name, const TypeInfo* type);
	bool try_add(std::string name, const TypeInfo* type);
	bool has(const std::string& name) const;
	void erase(const std::string& name);
	const TypeInfo* at(const std::string& name) const;
};

class ClassDatabase {
	std::map<std::string, std::map<std::vector<const TypeInfo*>, std::unique_ptr<const TypeInfo>>> types; //pattern name -> params -> type_info
	std::map<std::string, const TypeInfo*> current_class_parameters;
	std::map<std::string, std::pair<const ast::Class*, std::shared_ptr<const std::string>>> patterns; //name -> ast, filename
	std::optional<const TypeInfo*> get(const std::string& full_name) const;
	std::vector<TypeError> optional_errors;
	std::vector<const RealClassInfo*> classes_order;
	std::vector<std::unique_ptr<const RealClassInfo>> temporary_types_with_unknown_parameters;
public:
	class NonOwnedParameter {
		friend class ClassDatabase;
		ClassDatabase* cd;
		std::string name;
		NonOwnedParameter(ClassDatabase* cd, std::string name, const TypeInfo* type);
	public:
		NonOwnedParameter() = delete;
		NonOwnedParameter(const NonOwnedParameter&) = delete;
		NonOwnedParameter(NonOwnedParameter&& o);
		~NonOwnedParameter();
	};
	friend class NonOwnedParameter;
	ClassDatabase();
	std::vector<TypeError>& get_optional_errors();
	ClassDatabase(const ClassDatabase&) = delete;
	ClassDatabase(ClassDatabase&&) = default;
	ClassDatabase& operator=(ClassDatabase&&) = default;
	const TypeInfo* get_for_null() const;
	std::optional<std::vector<TypeError>> insert_pattern(const ast::Class* p, std::shared_ptr<const std::string> filename);
	const TypeInfo* get_for_int() const;
	const TypeInfo* get_for_void() const;
	std::optional<const TypeInfo*> get_for_string() const;
	std::variant<const TypeInfo*, std::vector<TypeError>> get(const ast::Identifier& t);
	std::variant<const TypeInfo*, std::vector<TypeError>> get(const ast::Type& t);
	std::variant<const TypeInfo*, std::vector<TypeError>> get(const ast::VariableType& t);
	std::variant<const TypeInfo*, std::vector<TypeError>> get(const ast::PointerType& t);
	std::variant<const TypeInfo*, std::vector<TypeError>> get_functional(size_t args_begin, size_t args_end, const TypeInfo* return_type, const std::vector<const TypeInfo*>& arguments);
	[[nodiscard]] NonOwnedParameter add_parameter(std::string name, const TypeInfo*);
	const std::vector<const RealClassInfo*>& get_ordered() const;
};

class RealFunctionInfo : public FunctionInfo {
	const std::string name;
	const TypeInfo* return_type;
	const std::unique_ptr<const TypeInfo> type_info_data;
	const std::vector<const TypeInfo*> arguments;
	const ast::Function* declaration_ast;
	const std::optional<const ast::Function*> body_ast;
	const std::optional<std::pair<const std::pair<std::vector<std::pair<ast::VariableType, ast::Identifier>>, std::unique_ptr<ast::statement::Block>>*, std::vector<const TypeInfo*>>> dual;
	RealFunctionInfo(std::string name, const TypeInfo* return_type, std::vector<const TypeInfo*> arguments, const ast::Function* declaration_ast, std::optional<const ast::Function*> body_ast, std::optional<std::pair<const std::pair<std::vector<std::pair<ast::VariableType, ast::Identifier>>, std::unique_ptr<ast::statement::Block>>*, std::vector<const TypeInfo*>>> dual, bool nojournal_marks);
public:
	bool nojournal_marks;

	RealFunctionInfo() = delete;
	const TypeInfo* type_info() const override;
	static std::variant<std::unique_ptr<RealFunctionInfo>, std::vector<TypeError>> make_function_declaration(std::optional<const RealFunctionInfo*> overridden, ClassDatabase& class_database, const ast::Function* ast, bool comes_from_nojournal);
	virtual std::optional<ast::FunctionKind> kind() const override;
	virtual std::optional<const TypeInfo*> call_with(const std::vector<const TypeInfo*>& args) const override;
	bool fully_defined() const;
	virtual std::string full_name() const override;
	std::optional<std::vector<TypeError>> check_body(VariableMap& variable_map, ClassDatabase& class_database) const;
	virtual ~RealFunctionInfo() = default;
	friend struct RealFunctionInfoView;
};

struct RealFunctionInfoView {
	const std::string& name;
	const TypeInfo* const return_type;
	const TypeInfo* const type_info_data;
	const std::vector<const TypeInfo*>& arguments;
	const ast::Function* const declaration_ast;
	const std::optional<const ast::Function*>& body_ast;
	const std::optional<std::pair<const std::pair<std::vector<std::pair<ast::VariableType, ast::Identifier>>, std::unique_ptr<ast::statement::Block>>*, std::vector<const TypeInfo*>>>& dual;
	RealFunctionInfoView() = delete;
	RealFunctionInfoView(const RealFunctionInfo&);
};

class RealClassInfo : public TypeInfo {
	const std::vector<const TypeInfo*> parameters;
	const std::optional<const TypeInfo*> superclass;
	std::map<std::string, const TypeInfo*> variables;
	std::map<std::string, std::unique_ptr<RealFunctionInfo>> functions;
	std::map<size_t, std::vector<const TypeInfo*>> constructors;
	const ast::Class* ast_data;
	RealClassInfo() = delete;
	RealClassInfo(std::vector<const TypeInfo*> parameters, std::optional<const TypeInfo*> superclass, const ast::Class* ast_data);

	virtual bool implicitly_convertible_from(const RealClassInfo* o) const override;
	virtual bool implicitly_convertible_from(const NullTypeInfo*) const override;
	virtual bool implicitly_convertible_from(const IntTypeInfo*) const override;
	virtual bool implicitly_convertible_from(const TemplateUnknownTypeInfo*) const override;
public:
	virtual std::optional<const TypeInfo*> get_superclass() const override;
	virtual ~RealClassInfo() = default;
	virtual bool constructible_with(const std::vector<const TypeInfo*>& args) const override;
	const std::string& pattern_name() const;
	virtual std::optional<const TypeInfo*> get_member(const std::string& name) const override;
	virtual bool check_nojournal_mark(bool mark) const override;
	virtual bool extendable() const override;
	static std::variant<std::pair<std::unique_ptr<RealClassInfo>, std::vector<TypeError>>, std::vector<TypeError>> make_class(ClassDatabase& class_database, const ast::Class* ast);
	std::optional<const RealFunctionInfo*> get_function(const std::string& name) const;
	virtual bool implicitly_convertible_to(const TypeInfo* o) const override;
	virtual std::string full_name() const override;
	bool has_unknown_parameters() const;
	bool comes_from_same_pattern(const RealClassInfo& o) const;
	friend struct RealClassInfoView;
	bool has_nojournal_mark() const;
};

struct RealClassInfoView {
	const std::vector<const TypeInfo*>& parameters;
	const std::optional<const TypeInfo*>& superclass;
	const std::map<std::string, const TypeInfo*>& variables;
	const std::map<std::string, std::unique_ptr<RealFunctionInfo>>& functions;
	const std::map<size_t, std::vector<const TypeInfo*>>& constructors;
	const ast::Class* const ast_data;

	RealClassInfoView() = delete;
	RealClassInfoView(const RealClassInfo& rci);
};

class IntTypeInfo : public TypeInfo {
	const std::map<std::string, std::unique_ptr<BuiltinIntFunctionInfo>> funs;
	virtual bool implicitly_convertible_from(const RealClassInfo*) const override;
	virtual bool implicitly_convertible_from(const NullTypeInfo*) const override;
	virtual bool implicitly_convertible_from(const IntTypeInfo*) const override;
	virtual bool implicitly_convertible_from(const TemplateUnknownTypeInfo*) const override;
	static std::map<std::string, std::unique_ptr<BuiltinIntFunctionInfo>> get_funs_map(const IntTypeInfo* int_type);
public:
	IntTypeInfo();
	virtual bool check_nojournal_mark(bool mark) const override;
	virtual bool implicitly_convertible_to(const TypeInfo* o) const override;
	virtual std::string full_name() const override;
	virtual std::optional<const TypeInfo*> get_member(const std::string& name) const override;
	virtual ~IntTypeInfo() = default;
};

class VoidTypeInfo : public TypeInfo {
	virtual bool implicitly_convertible_from(const RealClassInfo*) const override;
	virtual bool implicitly_convertible_from(const NullTypeInfo*) const override;
	virtual bool implicitly_convertible_from(const IntTypeInfo*) const override;
	virtual bool implicitly_convertible_from(const TemplateUnknownTypeInfo*) const override;
	virtual bool implicitly_convertible_from(const VoidTypeInfo*) const override;
public:
	virtual bool check_nojournal_mark(bool mark) const override;
	virtual bool implicitly_convertible_to(const TypeInfo* o) const override;
	virtual std::string full_name() const override;
	virtual std::optional<const TypeInfo*> get_member(const std::string&) const override;
	virtual ~VoidTypeInfo() = default;
};

std::pair<std::pair<std::vector<TypeError>, std::vector<TypeError>>, ClassDatabase> typecheck(const std::vector<ParsedModule>&);

}

#endif
