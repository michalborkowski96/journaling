#include "ast.h"

using namespace ast;
using namespace expression;
using namespace statement;

AstNode::AstNode(size_t begin, size_t end) : begin(begin), end(end) {}

Constructor::Constructor(size_t begin, size_t end, std::vector<std::pair<VariableType, Identifier>>&& arguments, std::optional<std::vector<Expression>>&& superclass_call, std::unique_ptr<Block>&& body) : AstNode(begin, end), arguments(move(arguments)), superclass_call(move(superclass_call)), body(move(body)){}

Destructor::Destructor(size_t begin, size_t end, std::unique_ptr<Block>&& body) : AstNode(begin, end), body(move(body)){}

Identifier::Identifier(size_t begin, size_t end, std::string&& name) : AstNode(begin, end), name(std::move(name)) {}

IntType::IntType(size_t begin, size_t end) : AstNode(begin, end){}

Null::Null(size_t begin, size_t end) : AstNode(begin, end){}

This::This(size_t begin, size_t end) : AstNode(begin, end){}

VoidType::VoidType(size_t begin, size_t end) : AstNode(begin, end){}

PointerType::PointerType(size_t begin, size_t end, Identifier&& name, std::vector<VariableType>&& parameters) : AstNode(begin, end), name(std::move(name)), parameters(std::move(parameters)){}

StringLiteral::StringLiteral(size_t begin, size_t end, std::string&& value) : AstNode(begin, end), value(std::move(value)) {}

New::New(size_t begin, size_t end, PointerType&& type, std::vector<Expression>&& args) : AstNode(begin, end), type(std::move(type)), args(std::move(args)) {}

IntegerLiteral::IntegerLiteral(size_t begin, size_t end, Integer value) : AstNode(begin, end), value(value) {}

MemberAccess::MemberAccess(size_t begin, size_t end, Expression object, Identifier&& member) : AstNode(begin, end), object(std::move(object)), member(std::move(member)) {}

FunctionCall::FunctionCall(size_t begin, size_t end, Expression function, std::vector<Expression>&& arguments, bool lazy) : AstNode(begin, end), function(std::move(function)), arguments(std::move(arguments)), lazy(lazy) {}

Cast::Cast(size_t begin, size_t end, PointerType&& target_type, Expression value) : AstNode(begin, end), target_type(std::move(target_type)), value(std::move(value)) {}

Negation::Negation(size_t begin, size_t end, Expression value, bool boolean) : AstNode(begin, end), value(std::move(value)), boolean(boolean) {}

BinaryOperation::BinaryOperation(size_t begin, size_t end, Expression left, Expression right, BinaryOperationType type) : AstNode(begin, end), left(std::move(left)), right(std::move(right)), type(type) {}

Block::Block(size_t begin, size_t end, std::vector<Statement>&& statements) : AstNode(begin, end), statements(std::move(statements)) {}

Empty::Empty(size_t begin, size_t end) : AstNode(begin, end) {}

VariableDeclaration::VariableDeclaration(size_t begin, size_t end, VariableType&& type, Identifier&& name, std::optional<Expression>&& value) : AstNode(begin, end), type(std::move(type)), name(std::move(name)), value(std::move(value)) {}

VariableAuto::VariableAuto(size_t begin, size_t end, Identifier&& name, expression::Expression value) : AstNode(begin, end), name(std::move(name)), value(std::move(value)) {}

Assignment::Assignment(size_t begin, size_t end, Expression variable, Expression value) : AstNode(begin, end), variable(std::move(variable)), value(std::move(value)) {}

Return::Return(size_t begin, size_t end, std::vector<Expression>&& values) : AstNode(begin, end), values(std::move(values)) {}

If::If(size_t begin, size_t end, Expression condition, std::unique_ptr<Block>&& body, std::optional<std::unique_ptr<Block>> body_else) : AstNode(begin, end), condition(std::move(condition)), body(std::move(body)), body_else(std::move(body_else)) {}

For::For(size_t begin, size_t end, Statement before, Expression condition, Statement after, std::unique_ptr<Block> body) : AstNode(begin, end), before(move(before)), condition(move(condition)), after(move(after)), body(std::move(body)) {}

Break::Break(size_t begin, size_t end) : AstNode(begin, end) {}

Continue::Continue(size_t begin, size_t end) : AstNode(begin, end) {}

While::While(size_t begin, size_t end, Expression condition, std::unique_ptr<Block>&& body) : AstNode(begin, end), condition(std::move(condition)), body(std::move(body)) {}

StatementExpression::StatementExpression(size_t begin, size_t end, Expression expression) : AstNode(begin, end), expression(std::move(expression)) {}

Function::Function(size_t begin, size_t end, Type&& return_type, Identifier&& name, std::vector<std::pair<VariableType, Identifier>>&& arguments, std::optional<std::unique_ptr<Block>>&& body, std::optional<FunctionKind> kind, std::optional<std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<Block>>>&& dual) : AstNode(begin, end), return_type(std::move(return_type)), name(std::move(name)), arguments(std::move(arguments)), body(std::move(body)), kind(kind), dual(std::move(dual)){}

ClassVariableInteger::ClassVariableInteger(size_t begin, size_t end, Identifier&& name) : AstNode(begin, end), name(std::move(name)) {}

ClassVariablePointer::ClassVariablePointer(size_t begin, size_t end, Identifier&& name, PointerType&& type, bool strong) : AstNode(begin, end), name(std::move(name)), type(std::move(type)), strong(strong) {}

Class::Class(size_t begin, size_t end, Identifier&& name, std::vector<Function>&& functions, std::vector<std::vector<Function>>&& optional_functions, std::vector<ClassVariable>&& variables, std::vector<Identifier>&& parameters, std::optional<PointerType>&& superclass, std::vector<Constructor>&& constructors, std::optional<Destructor>&& destructor, bool public_variables, bool abstract, bool nojournal) : AstNode(begin, end), name(std::move(name)), functions(std::move(functions)), optional_functions(std::move(optional_functions)), variables(std::move(variables)), parameters(move(parameters)), superclass(move(superclass)), constructors(move(constructors)), destructor(move(destructor)), public_variables(public_variables), abstract(abstract), nojournal(nojournal) {}

Module::Module(std::vector<Import>&& imports, std::vector<Class>&& classes) : imports(std::move(imports)), classes(std::move(classes)) {}

Module::Import::Import(std::string path, bool standard) : path(std::move(path)), standard(standard) {}
