#ifndef AST_H
#define AST_H

#include <variant>
#include <string>
#include <vector>
#include <memory>
#include <optional>

#include "util.h"

namespace ast {

	struct AstNode {
		size_t begin;
		size_t end;
		AstNode() = delete;
		AstNode(AstNode&&) = default;
		AstNode& operator=(AstNode&&) = default;
	protected:
		AstNode(size_t begin, size_t end);
	};

	struct Identifier : public AstNode {
		std::string name;
		Identifier(size_t begin, size_t end, std::string&& name);
	};

	struct IntType;

	struct VoidType;

	struct PointerType;

	using VariableType = std::variant<IntType, PointerType>;

	using Type = std::variant<VoidType, VariableType>;

	template<typename A, typename B, typename C>

	auto visit_type (const Type& t, A a, B b, C c){
		return std::visit(overload(
		[&](const VoidType& t){return a(t);},
		[&](const VariableType& t){return std::visit(overload(
			[&](const IntType& t){return b(t);},
			[&](const PointerType& t){return c(t);}
		), t);}
		), t);
	}

	struct IntType : public AstNode {
		IntType(size_t begin, size_t end);
	};

	struct VoidType : public AstNode {
		VoidType(size_t begin, size_t end);
	};

	struct PointerType : public AstNode {
		Identifier name;
		std::vector<VariableType> parameters;
		PointerType(size_t begin, size_t end, Identifier&& name, std::vector<VariableType>&& parameters);
	};

	namespace expression {

		struct StringLiteral;

		struct IntegerLiteral;

		struct MemberAccess;

		struct FunctionCall;

		struct LazyFunctionCall;

		struct Cast;

		struct Null;

		struct This;

		struct Negation;

		struct BinaryOperation;

		struct New;

		using Expression = Wrap<std::variant, std::unique_ptr, StringLiteral, IntegerLiteral, Identifier, New, Negation, Cast, Null, This, MemberAccess, FunctionCall, LazyFunctionCall, BinaryOperation>;

		struct StringLiteral : public AstNode {
			std::string value;
			StringLiteral(size_t begin, size_t end, std::string&& value);
		};

		struct New : public AstNode {
			PointerType type;
			std::vector<Expression> args;
			New(size_t begin, size_t end, PointerType&& type, std::vector<Expression>&& args);
		};

		struct IntegerLiteral : public AstNode {
			Integer value;
			IntegerLiteral(size_t begin, size_t end, Integer value);
		};

		struct Null : public AstNode {
			Null(size_t begin, size_t end);
		};

		struct This : public AstNode {
			This(size_t begin, size_t end);
		};

		struct MemberAccess : public AstNode {
			Expression object;
			Identifier member;
			MemberAccess(size_t begin, size_t end, Expression object, Identifier&& member);
		};

		struct FunctionCall : public AstNode {
			Expression function;
			std::vector<Expression> arguments;
			FunctionCall(size_t begin, size_t end, Expression function, std::vector<Expression>&& arguments);
		};

		struct LazyFunctionCall : public AstNode {
			Expression function;
			std::vector<std::pair<Expression, bool>> arguments;
			LazyFunctionCall(size_t begin, size_t end, Expression function, std::vector<std::pair<Expression, bool>>&& arguments);
		};

		struct Cast : public AstNode {
			PointerType target_type;
			Expression value;
			Cast(size_t begin, size_t end, PointerType&& target_type, Expression value);
		};

		struct Negation : public AstNode {
			Expression value;
			bool boolean;
			Negation(size_t begin, size_t end, Expression value, bool boolean);
		};

		enum class BinaryOperationType {
			MUL, DIV, ADD, SUB, OR, AND, MOD, LESS, LESS_EQUAL, MORE, MORE_EQUAL, EQUAL, NOT_EQUAL
		};

		struct BinaryOperation : public AstNode {
			Expression left;
			Expression right;
			BinaryOperationType type;
			BinaryOperation(size_t begin, size_t end, Expression left, Expression right, BinaryOperationType type);
		};

	}

	namespace statement {

		struct Block;

		struct Empty;

		struct VariableDeclaration;

		struct VariableAuto;

		struct Assignment;

		struct Return;

		struct If;

		struct While;

		struct For;

		struct Break;

		struct Continue;

		struct StatementExpression;

		using Statement = Wrap<std::variant, std::unique_ptr, Break, VariableAuto, Continue, Block, Empty, For, VariableDeclaration, Return, If, While, Assignment, StatementExpression>;

		struct Block : AstNode  {
			std::vector<Statement> statements;
			Block(size_t begin, size_t end, std::vector<Statement>&& statements);
		};

		struct Empty : AstNode  {
			Empty(size_t begin, size_t end);
		};

		struct VariableDeclaration : public AstNode {
			VariableType type;
			Identifier name;
			std::optional<expression::Expression> value;
			VariableDeclaration(size_t begin, size_t end, VariableType&& type, Identifier&& name, std::optional<expression::Expression>&& value);
		};

		struct VariableAuto : public AstNode {
			Identifier name;
			expression::Expression value;
			VariableAuto(size_t begin, size_t end, Identifier&& name, expression::Expression value);
		};

		struct Assignment : public AstNode {
			expression::Expression variable;
			expression::Expression value;
			Assignment(size_t begin, size_t end, expression::Expression variable, expression::Expression value);
		};

		struct Return : public AstNode {
			std::vector<expression::Expression> values;
			Return(size_t begin, size_t end, std::vector<expression::Expression>&& values);
		};

		struct If : public AstNode {
			expression::Expression condition;
			std::unique_ptr<Block> body;
			std::optional<std::unique_ptr<Block>> body_else;
			If(size_t begin, size_t end, expression::Expression condition, std::unique_ptr<Block>&& body, std::optional<std::unique_ptr<Block>> body_else);
		};

		struct For : public AstNode {
			Statement before;
			expression::Expression condition;
			Statement after;
			std::unique_ptr<Block> body;
			For(size_t begin, size_t end, Statement before, expression::Expression condition, Statement after, std::unique_ptr<Block> body);
		};

		struct Break : AstNode  {
			Break(size_t begin, size_t end);
		};

		struct Continue : AstNode  {
			Continue(size_t begin, size_t end);
		};

		struct While : public AstNode {
			expression::Expression condition;
			std::unique_ptr<Block> body;
			While(size_t begin, size_t end, expression::Expression condition, std::unique_ptr<Block>&& body);
		};

		struct StatementExpression : public AstNode {
			expression::Expression expression;
			StatementExpression(size_t begin, size_t end, expression::Expression expression);
		};
	}

	enum class FunctionKind {
		DUAL, NOEFFECT, IRREVERSIBLE
	};

	struct Constructor : public AstNode {
		std::vector<std::pair<VariableType, Identifier>> arguments;
		std::optional<std::vector<expression::Expression>> superclass_call;
		std::unique_ptr<statement::Block> body;
		Constructor(size_t begin, size_t end, std::vector<std::pair<VariableType, Identifier>>&& arguments, std::optional<std::vector<expression::Expression>>&& superclass_call, std::unique_ptr<statement::Block>&& body);
	};

	struct Destructor : public AstNode {
		std::unique_ptr<statement::Block> body;
		Destructor(size_t begin, size_t end, std::unique_ptr<statement::Block>&& body);
	};

	struct Function : public AstNode {
		Type return_type;
		Identifier name;
		std::vector<std::pair<VariableType, Identifier>> arguments;
		std::optional<std::unique_ptr<statement::Block>> body;
		std::optional<FunctionKind> kind;
		std::optional<std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<statement::Block>>> dual;
		Function(size_t begin, size_t end, Type&& return_type, Identifier&& name, std::vector<std::pair<VariableType, Identifier>>&& arguments, std::optional<std::unique_ptr<statement::Block>>&& body, std::optional<FunctionKind> kind, std::optional<std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<statement::Block>>>&& dual);
	};

	struct ClassVariableInteger : public AstNode {
		Identifier name;
		ClassVariableInteger(size_t begin, size_t end, Identifier&& name);
	};

	struct ClassVariablePointer : public AstNode {
		Identifier name;
		PointerType type;
		bool strong;
		ClassVariablePointer(size_t begin, size_t end, Identifier&& name, PointerType&& type, bool strong);
	};

	using ClassVariable = std::variant<ClassVariableInteger, ClassVariablePointer>;

	struct Class : public AstNode {
		Identifier name;
		std::vector<Function> functions;
		std::vector<std::vector<Function>> optional_functions;
		std::vector<ClassVariable> variables;
		std::vector<Identifier> parameters;
		std::optional<PointerType> superclass;
		std::vector<Constructor> constructors;
		std::optional<Destructor> destructor;
		bool public_variables;
		bool abstract;
		bool nojournal;
		Class(size_t begin, size_t end, Identifier&& name, std::vector<Function>&& functions, std::vector<std::vector<Function>>&& optional_functions, std::vector<ClassVariable>&& variables, std::vector<Identifier>&& parameters, std::optional<PointerType>&& superclass, std::vector<Constructor>&& constructors, std::optional<Destructor>&& destructor, bool public_variables, bool abstract, bool nojournal);
	};

	struct Module {
		std::vector<std::string> imports;
		std::vector<Class> classes;
		Module() = delete;
		Module(std::vector<std::string>&& imports, std::vector<Class>&& classes);
	};

}

#endif
