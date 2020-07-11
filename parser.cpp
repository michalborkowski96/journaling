#include "parser.h"
#include "system.h"

#include <fstream>
#include <functional>
#include <map>
#include <set>

using namespace lexer;
using namespace ast;
using namespace expression;
using namespace statement;

namespace {
	class TokensView {
		const std::vector<Lexeme>& tokens;
		const Lexeme end;
		TokensView() = delete;
	public:
		const Lexeme& operator[](size_t pos) const {
			return pos < tokens.size() ? tokens[pos] : end;
		}
		const std::string& module;
		TokensView(const std::vector<Lexeme>& tokens, const std::string& module) : tokens(tokens), end(Lexeme(tokens.size(), tokens.size(), LexemeType::END_OF_FILE)), module(module) {}
	};

	class Parser {
	public:
		struct Error : std::exception {
			const size_t begin;
			const size_t end;
			const char* what() const noexcept {
				return "Parser error";
			}
			Error() = delete;
			Error(size_t begin, size_t end) : begin(begin), end(end) {}
		};
	private:
		size_t pos;
		const TokensView& tokens;
		Parser() = delete;

		static const std::map<LexemeType, int> priorities;

		Error error() const {
			return Error(tokens[pos].begin, tokens[pos].end);
		}

		bool expect() {
			return false;
		}

		template<size_t i, typename T, typename std::enable_if_t<!std::is_enum<T>::value, int> = 0, typename... Ts>
		bool expect(const T& fun, Ts...) {
			return fun(), true;
		}

		bool expect_skip() {
			return false;
		}

		template<typename T, typename... Ts>
		bool expect_skip(const T&, const Ts&... args) {
			if(std::is_enum<T>::value) {
				return expect_skip(args...);
			} else {
				return expect(args...);
			}
		}

		template<size_t i, typename... Ts>
		bool expect(LexemeType expected_token, const Ts&... expected_tokens) {
			if(tokens[pos + i].type != expected_token) {
				return expect_skip(expected_tokens...);
			}
			return expect<i + 1>(expected_tokens...);
		}

		template<typename... Ts>
		bool expect(const Ts&... expected_tokens) {
			return expect<0>(expected_tokens...);
		}

		std::vector<Expression> parse_call_arguments(bool expect_round_bracket) {
			check_for_tokens(expect_round_bracket ? LexemeType::BRACKET_OPEN : LexemeType::SQUARE_OPEN);
			++pos;
			std::vector<Expression> args;
			while(tokens[pos].type != (expect_round_bracket ? LexemeType::BRACKET_CLOSE : LexemeType::SQUARE_CLOSE)) {
				args.push_back(parse_expression());
				if(tokens[pos].type == LexemeType::COMMA && tokens[pos + 1].type != (expect_round_bracket ? LexemeType::BRACKET_CLOSE : LexemeType::SQUARE_CLOSE)) {
					++pos;
				}
			}
			++pos;
			return args;
		}

		Expression parse_ll_expression() {
			std::optional<Expression> expression;
			size_t begin = tokens[pos].begin;
			expect(
			LexemeType::SQUARE_OPEN, [&](){
				std::vector<std::pair<Identifier, Lambda::CaptureType>> capture;
				++pos;
				if(tokens[pos].type != LexemeType::SQUARE_CLOSE) {
					while(true) {
						std::string capture_type = parse_identifier().name;
						Lambda::CaptureType ct;
						if(capture_type == "j") {
							ct = Lambda::CaptureType::JOURNAL;
						} else if (capture_type == "r") {
							ct = Lambda::CaptureType::REFERENCE;
						} else {
							--pos;
							throw error();
						}
						capture.emplace_back(parse_identifier(), ct);
						if(tokens[pos].type == LexemeType::COMMA) {
							++pos;
							continue;
						}
						if(tokens[pos].type == LexemeType::SQUARE_CLOSE) {
							break;
						}
						throw error();
					}
				}
				++pos;
				auto args = parse_function_declaration_args();
				VariableType type = parse_variable_type();
				std::unique_ptr<Block> body = std::make_unique<Block>(parse_block());
				expression = std::make_unique<Lambda>(begin, tokens[pos - 1].end, std::move(capture), std::move(args), std::move(type), std::move(body));
			},
			LexemeType::STRING_LITERAL, [&]() {
				std::string str = parse_string();
				expression = std::make_unique<StringLiteral>(begin, tokens[pos - 1].end, move(str));
			},
			LexemeType::INT_LITERAL, [&]() {
				static_assert(std::is_same<Integer, long long>::value || std::is_same<Integer, long>::value);
				if (std::is_same<Integer, long long>::value) {
					long long val;
					try {
						val = std::stoll(tokens.module.substr(tokens[pos].begin, tokens[pos].end - tokens[pos].begin));
					} catch (...) {
						throw error();
					}
					++pos;
					expression = std::make_unique<IntegerLiteral>(begin, tokens[pos - 1].end, val);
				} else if (std::is_same<Integer, long>::value) {
					long val;
					try {
						val = std::stol(tokens.module.substr(tokens[pos].begin, tokens[pos].end - tokens[pos].begin));
					} catch (...) {
						throw error();
					}
					++pos;
					expression = std::make_unique<IntegerLiteral>(begin, tokens[pos - 1].end, val);
				} else {
					throw std::runtime_error("Internal error.");
				}
			},
			LexemeType::BYTE_LITERAL, [&]() {
				expression = std::make_unique<IntegerLiteral>(begin, tokens[pos].end, tokens.module[tokens[pos].end - 2]);
				++pos;
			},
			LexemeType::IDENTIFIER, [&]() {
				expression = std::make_unique<Identifier>(parse_identifier());
			},
			LexemeType::BRACKET_OPEN, [&]() {
				++pos;
				expression = parse_expression();
				check_for_tokens(LexemeType::BRACKET_CLOSE);
				++pos;
			},
			LexemeType::NEW, [&]() {
				++pos;
				PointerType type = parse_pointer_type();
				std::vector<Expression> arguments = parse_call_arguments(true);
				expression = std::make_unique<New>(begin, tokens[pos - 1].end, std::move(type), move(arguments));
			},
			LexemeType::SUB, [&]() {
				++pos;
				Expression expr = parse_ll_expression();
				expression = std::make_unique<Negation>(begin, tokens[pos - 1].end, move(expr), false);
			},
			LexemeType::NEGATION, [&]() {
				++pos;
				Expression expr = parse_ll_expression();
				expression = std::make_unique<Negation>(begin, tokens[pos - 1].end, move(expr), true);
			},
			LexemeType::CAST, [&]() {
				++pos;
				check_for_tokens(LexemeType::LESS);
				++pos;
				PointerType type = parse_pointer_type();
				check_for_tokens(LexemeType::MORE);
				++pos;
				check_for_tokens(LexemeType::BRACKET_OPEN);
				Expression expr = parse_ll_expression();
				expression = std::make_unique<Cast>(begin, tokens[pos - 1].end, std::move(type), std::move(expr));
			},
			LexemeType::NIL, [&]() {
				++pos;
				expression = std::make_unique<Null>(begin, tokens[pos - 1].end);
			},
			LexemeType::THIS, [&]() {
				++pos;
				expression = std::make_unique<This>(begin, tokens[pos - 1].end);
			}
			);
			if(!expression) {
				throw error();
			}
			begin = tokens[pos].begin;
			while(
				expect(
				LexemeType::BRACKET_OPEN, [&](){
					std::vector<Expression> args = parse_call_arguments(true);
					expression = std::make_unique<FunctionCall>(begin, tokens[pos - 1].end, std::move(*expression), move(args), false);
				},
				LexemeType::SQUARE_OPEN, [&](){
					std::vector<Expression> args = parse_call_arguments(false);
					expression = std::make_unique<FunctionCall>(begin, tokens[pos - 1].end, std::move(*expression), move(args), true);
				},
				LexemeType::DOT, [&](){
					++pos;
					Identifier name = parse_identifier();
					expression = std::make_unique<MemberAccess>(begin, tokens[pos - 1].end, std::move(*expression), std::move(name));
				}
				))
			{}
			return std::move(*expression);
		}

		struct ParsingBinaryOperation {
			Expression expr;
			std::optional<std::pair<LexemeType, std::unique_ptr<ParsingBinaryOperation>>> next;
			ParsingBinaryOperation() = delete;
			ParsingBinaryOperation(Expression&& expr) : expr(std::move(expr)) {}

			void join_with_next() {
				static const std::map<LexemeType, BinaryOperationType> operators = {
					{LexemeType::MUL, BinaryOperationType::MUL},
					{LexemeType::DIV, BinaryOperationType::DIV},
					{LexemeType::ADD, BinaryOperationType::ADD},
					{LexemeType::SUB, BinaryOperationType::SUB},
					{LexemeType::OR, BinaryOperationType::OR},
					{LexemeType::AND, BinaryOperationType::AND},
					{LexemeType::MOD, BinaryOperationType::MOD},
					{LexemeType::LESS, BinaryOperationType::LESS},
					{LexemeType::LE, BinaryOperationType::LESS_EQUAL},
					{LexemeType::MORE, BinaryOperationType::MORE},
					{LexemeType::ME, BinaryOperationType::MORE_EQUAL},
					{LexemeType::EQ, BinaryOperationType::EQUAL},
					{LexemeType::NE, BinaryOperationType::NOT_EQUAL},
				};
				size_t begin = std::visit([&](auto& st){return st->begin;}, expr);
				size_t end = std::visit([&](auto& st){return st->end;}, next->second->expr);
				expr = std::make_unique<BinaryOperation>(begin, end, std::move(expr), std::move(next->second->expr), operators.at(next->first));
				next = move(next->second->next);
			}
		};

		Expression parse_expression() {
			static const std::set<LexemeType> delimiters = [](){
				std::set<LexemeType> delimiters;
				for(const auto& [type, priority] : priorities) {
					delimiters.insert(type);
				}
				return delimiters;
			}();
			ParsingBinaryOperation pbo(parse_ll_expression());
			ParsingBinaryOperation* end =  &pbo;
			while(delimiters.find(tokens[pos].type) != delimiters.end()) {
				LexemeType t = tokens[pos].type;
				++pos;
				end->next = make_pair(t, std::make_unique<ParsingBinaryOperation>(parse_ll_expression()));
				end = end->next->second.get();
			}

			while(pbo.next) {
				int lowest_priority = priorities.at(pbo.next->first);
				ParsingBinaryOperation* pb = pbo.next->second.get();
				while(pb->next) {
					lowest_priority = std::min(lowest_priority, priorities.at(pb->next->first));
					pb = pb->next->second.get();
				}
				pb = &pbo;
				while(pb->next) {
					if(priorities.at(pb->next->first) == lowest_priority) {
						pb->join_with_next();
					} else {
						pb = pb->next->second.get();
					}
				}
			}
			return std::move(pbo.expr);
		}

		Statement parse_statement(bool eat_semicolon = true) {
			std::optional<Statement> s;
			size_t begin = tokens[pos].begin;
			switch(tokens[pos].type) {
			case LexemeType::BREAK:
				s = std::make_unique<Break>(begin, tokens[pos].end);
				++pos;
				break;
			case LexemeType::CONTINUE:
				s = std::make_unique<Continue>(begin, tokens[pos].end);
				++pos;
				break;
			case LexemeType::CURLY_OPEN:
				return std::make_unique<Block>(parse_block());
			case LexemeType::SEMICOLON:
				s = std::make_unique<Empty>(begin, tokens[pos].end);
				break;
			case LexemeType::FOR:
			{
				++pos;
				check_for_tokens(LexemeType::BRACKET_OPEN);
				++pos;
				Statement before = parse_statement();
				if(!std::holds_alternative<std::unique_ptr<VariableDeclaration>>(before)
						&& !std::holds_alternative<std::unique_ptr<Empty>>(before)
						&& !std::holds_alternative<std::unique_ptr<VariableAuto>>(before)
						&& !std::holds_alternative<std::unique_ptr<Assignment>>(before)
						&& !std::holds_alternative<std::unique_ptr<StatementExpression>>(before)) {
					throw error();
				}
				Expression condition = parse_expression();
				check_for_tokens(LexemeType::SEMICOLON);
				++pos;
				Statement after = tokens[pos].type == LexemeType::BRACKET_CLOSE ? std::make_unique<Empty>(tokens[pos - 1].end, tokens[pos].begin) : parse_statement(false);
				if(!std::holds_alternative<std::unique_ptr<Empty>>(after)
						&& !std::holds_alternative<std::unique_ptr<Assignment>>(after)
						&& !std::holds_alternative<std::unique_ptr<StatementExpression>>(after)) {
					throw error();
				}
				check_for_tokens(LexemeType::BRACKET_CLOSE);
				++pos;
				std::unique_ptr<Block> body = std::make_unique<Block>(parse_block());
				return std::make_unique<For>(begin, tokens[pos - 1].end, std::move(before), std::move(condition), std::move(after), std::move(body));
			}
			case LexemeType::VAR:
			{
				++pos;
				VariableType type = parse_variable_type();
				Identifier name = parse_identifier();
				std::optional<Expression> value;
				if(tokens[pos].type == LexemeType::ASSIGNMENT) {
					++pos;
					value = parse_expression();
				}
				s = std::make_unique<VariableDeclaration>(begin, tokens[pos - 1].end, std::move(type), std::move(name), std::move(value));
				break;
			}
			case LexemeType::AUTO:
			{
				++pos;
				Identifier name = parse_identifier();
				check_for_tokens(LexemeType::ASSIGNMENT);
				++pos;
				Expression value = parse_expression();
				s = std::make_unique<VariableAuto>(begin, tokens[pos - 1].end, std::move(name), std::move(value));
				break;
			}
			case LexemeType::RETURN:
			{
				if(!eat_semicolon) {
					throw error();
				}
				++pos;
				std::vector<Expression> exprs;
				if(tokens[pos].type != LexemeType::SEMICOLON) {
					exprs.push_back(parse_expression());
					while(tokens[pos].type == LexemeType::COMMA) {
						++pos;
						exprs.push_back(parse_expression());
					}
				}
				s = std::make_unique<Return>(begin, tokens[pos - 1].end, move(exprs));
				break;
			}
			case LexemeType::IF:
			{
				++pos;
				check_for_tokens(LexemeType::BRACKET_OPEN);
				++pos;
				Expression condition = parse_expression();
				check_for_tokens(LexemeType::BRACKET_CLOSE);
				++pos;
				std::unique_ptr<Block> body = std::make_unique<Block>(parse_block());
				std::optional<std::unique_ptr<Block>> body_else;
				if(tokens[pos].type == LexemeType::ELSE) {
					++pos;
					if(tokens[pos].type == LexemeType::IF) {
						size_t if_begin = tokens[pos].begin;
						std::vector<Statement> sv;
						sv.push_back(parse_statement());
						body_else = std::make_unique<Block>(if_begin, tokens[pos - 1].end, std::move(sv));
					} else {
						body_else = std::make_unique<Block>(parse_block());
					}
				}
				return std::make_unique<If>(begin, tokens[pos].end, std::move(condition), std::move(body), std::move(body_else));
			}
			case LexemeType::WHILE:
			{
				++pos;
				check_for_tokens(LexemeType::BRACKET_OPEN);
				++pos;
				Expression condition = parse_expression();
				check_for_tokens(LexemeType::BRACKET_CLOSE);
				++pos;
				std::unique_ptr<Block> body = std::make_unique<Block>(parse_block());
				return std::make_unique<While>(begin, tokens[pos].end, std::move(condition), std::move(body));
			}
			default:
			{
				Expression expr = parse_expression();
				if(tokens[pos].type == LexemeType::ASSIGNMENT) {
					++pos;
					Expression value = parse_expression();
					s = std::make_unique<Assignment>(begin, tokens[pos - 1].end, std::move(expr), std::move(value));
					break;
				} else {
					s = std::make_unique<StatementExpression>(begin, tokens[pos - 1].end, std::move(expr));
					if(!std::holds_alternative<std::unique_ptr<FunctionCall>>(expr)) {
						throw error();
					}
					break;
				}
			}
			}
			if(!s) {
				throw std::runtime_error("Internal error.");
			}
			if(eat_semicolon) {
				check_for_tokens(LexemeType::SEMICOLON);
				std::visit([&](auto& st){st->end = tokens[pos].end;}, *s);
				++pos;
			}
			return std::move(*s);
		}

		Block parse_block() {
			size_t begin = tokens[pos].begin;
			check_for_tokens(LexemeType::CURLY_OPEN);
			++pos;
			std::vector<Statement> statements;
			while(tokens[pos].type != LexemeType::CURLY_CLOSE) {
				statements.push_back(parse_statement());
			}
			size_t end = tokens[pos].end;
			++pos;
			return Block(begin, end, move(statements));
		}

		Identifier parse_identifier() {
			check_for_tokens(LexemeType::IDENTIFIER);
			Identifier i(tokens[pos].begin, tokens[pos].end, tokens.module.substr(tokens[pos].begin, tokens[pos].end - tokens[pos].begin));
			++pos;
			return i;
		}

		template<size_t i>
		void check_for_tokens() const {}

		template<size_t i, typename... Ts>
		void check_for_tokens(LexemeType t, Ts... args) const {
			if(tokens[pos + i].type != t) {
				throw error();
			}
			check_for_tokens<i + 1>(args...);
		}

		template<typename... Ts>
		void check_for_tokens(Ts... args) const {
			check_for_tokens<0>(args...);
		}

		VariableType parse_variable_type() {
			if(tokens[pos].type == LexemeType::INT) {
				IntType vt(tokens[pos].begin, tokens[pos].end);
				++pos;
				return vt;
			}
			return parse_pointer_type();
		}

		Type parse_type() {
			if(tokens[pos].type == LexemeType::VOID) {
				VoidType vt(tokens[pos].begin, tokens[pos].end);
				++pos;
				return vt;
			}
			return parse_variable_type();
		}

		std::vector<std::pair<VariableType, Identifier>> parse_function_declaration_args() {
			std::vector<std::pair<VariableType, Identifier>> arguments;
			check_for_tokens(LexemeType::BRACKET_OPEN);
			++pos;
			while(tokens[pos].type != LexemeType::BRACKET_CLOSE) {
				VariableType type = parse_variable_type();
				arguments.push_back(std::make_pair(move(type), parse_identifier()));
				if(tokens[pos].type != LexemeType::COMMA && tokens[pos].type != LexemeType::BRACKET_CLOSE) {
					throw error();
				}
				if(tokens[pos].type == LexemeType::COMMA) {
					++pos;
				}
			}
			++pos;
			return arguments;
		}

		Function parse_function() {
			size_t begin = tokens[pos].begin;
			check_for_tokens(LexemeType::FUN);
			++pos;
			Type return_type = parse_type();
			Identifier name = parse_identifier();

			std::vector<std::pair<VariableType, Identifier>> arguments = parse_function_declaration_args();

			std::optional<std::unique_ptr<Block>> body;
			if(tokens[pos].type == LexemeType::CURLY_OPEN) {
				body = std::make_unique<Block>(parse_block());
			}

			std::optional<FunctionKind> kind;
			std::optional<std::pair<std::vector<std::pair<VariableType, Identifier>>, std::unique_ptr<Block>>> dual;

			expect(LexemeType::NOEFFECT, [&](){kind = FunctionKind::NOEFFECT; ++pos;},
				LexemeType::AUTO, [&](){kind = FunctionKind::AUTO; ++pos;},
				LexemeType::DUAL, [&](){
					kind = FunctionKind::DUAL;
					++pos;
					if(tokens[pos].type == LexemeType::BRACKET_OPEN) {
						auto args = parse_function_declaration_args();
						dual = std::make_pair(move(args), std::make_unique<Block>(parse_block()));
					}
			});

			check_for_tokens(LexemeType::SEMICOLON);
			++pos;

			return Function(begin, tokens[pos - 1].end, std::move(return_type), std::move(name), move(arguments), move(body), move(kind), move(dual));
		}

		Destructor parse_destructor() {
			size_t begin = tokens[pos].begin;
			check_for_tokens(LexemeType::DESTRUCTOR);
			++pos;
			std::unique_ptr<Block> body = std::make_unique<Block>(parse_block());
			check_for_tokens(LexemeType::SEMICOLON);
			++pos;
			return Destructor(begin, tokens[pos - 1].end, std::move(body));
		}

		Constructor parse_constructor() {
			size_t begin = tokens[pos].begin;
			check_for_tokens(LexemeType::CONSTRUCTOR);
			++pos;
			std::vector<std::pair<VariableType, Identifier>> arguments = parse_function_declaration_args();
			std::optional<std::vector<expression::Expression>> superclass_call;
			if(tokens[pos].type == LexemeType::BRACKET_OPEN) {
				superclass_call = parse_call_arguments(true);
			}
			std::unique_ptr<Block> body = std::make_unique<Block>(parse_block());
			check_for_tokens(LexemeType::SEMICOLON);
			++pos;
			return Constructor(begin, tokens[pos - 1].end, std::move(arguments), std::move(superclass_call), std::move(body));
		}

		PointerType parse_pointer_type() {
			size_t begin = tokens[pos].begin;
			Identifier name = parse_identifier();
			std::vector<VariableType> parameters;
			if(tokens[pos].type == LexemeType::LESS) {
				++pos;
				while(tokens[pos].type != LexemeType::MORE) {
					parameters.push_back(parse_variable_type());
					if(tokens[pos].type == LexemeType::COMMA) {
						++pos;
					}
				}
				++pos;
			}
			return PointerType(begin, tokens[pos - 1].end, std::move(name), move(parameters));
		}

		Class parse_class() {
			size_t begin = tokens[pos].begin;
			bool public_variables = false;
			bool abstract = false;
			bool nojournal = false;

			if(tokens[pos].type == LexemeType::STRUCT) {
				public_variables = true;
				nojournal = true;
				++pos;
			} else {
				if(tokens[pos].type == LexemeType::ABSTRACT) {
					abstract = true;
					++pos;
				}
				if(tokens[pos].type == LexemeType::NOJOURNAL) {
					nojournal = true;
					++pos;
				}
			}

			Identifier name = parse_identifier();

			std::vector<Identifier> parameters;
			while(tokens[pos].type == LexemeType::IDENTIFIER) {
				parameters.push_back(parse_identifier());
			}

			std::optional<PointerType> superclass;
			if(tokens[pos].type == LexemeType::COLON) {
				++pos;
				superclass = parse_pointer_type();
			}

			check_for_tokens(LexemeType::CURLY_OPEN);
			++pos;

			std::vector<Function> functions;
			std::vector<std::vector<Function>> optional_functions;
			std::vector<Constructor> constructors;
			std::optional<Destructor> destructor;
			std::vector<ClassVariable> variables;

			auto variable_parser = [&](){
				size_t begin = tokens[pos].begin;
				bool strong = tokens[pos].type == LexemeType::STRONG;
				++pos;
				PointerType type = parse_pointer_type();
				check_for_tokens(LexemeType::IDENTIFIER, LexemeType::SEMICOLON);
				Identifier name = parse_identifier();
				++pos;
				variables.push_back(ClassVariablePointer(begin, tokens[pos - 1].end, std::move(name), std::move(type), strong));
			};

			while(true) {
				if(!expect(
					LexemeType::OPTIONAL, [&](){
						std::vector<Function> funs;
						++pos;
						check_for_tokens(LexemeType::CURLY_OPEN);
						++pos;
						while(tokens[pos].type != LexemeType::CURLY_CLOSE) {
							funs.push_back(parse_function());
						}
						++pos;
						optional_functions.push_back(move(funs));
					},
					LexemeType::FUN, [&](){
						functions.push_back(parse_function());
					},
					LexemeType::CONSTRUCTOR, [&](){
						constructors.push_back(parse_constructor());
					},
					LexemeType::DESTRUCTOR, [&](){
						if(destructor) {
							throw error();
						}
						destructor = parse_destructor();
					},
					LexemeType::INT, LexemeType::IDENTIFIER, LexemeType::SEMICOLON, [&](){
						size_t begin = tokens[pos].begin;
						++pos;
						Identifier name = parse_identifier();
						size_t end = tokens[pos].end;
						++pos;
						variables.push_back(ClassVariableInteger(begin, end, std::move(name)));
					},
					LexemeType::STRONG, variable_parser,
					LexemeType::WEAK, variable_parser
				)) {
					break;
				}
			}

			check_for_tokens(LexemeType::CURLY_CLOSE);
			++pos;

			return Class(begin, tokens[pos - 1].end, std::move(name), std::move(functions), std::move(optional_functions), std::move(variables), std::move(parameters), std::move(superclass), std::move(constructors), std::move(destructor), public_variables, abstract, nojournal);
		}

		std::string parse_string() {
			check_for_tokens(LexemeType::STRING_LITERAL);
			std::string data;
			for(size_t i = tokens[pos].begin + 1; i < tokens[pos].end - 1; ++i) {
				if(tokens.module[i] == '\\') {
					++i;
					if(i < tokens[pos].end) {
						data += tokens.module[i];
					}
				} else {
					data += tokens.module[i];
				}
			}
			++pos;
			return data;
		}

		Parser(const TokensView& tokens) : tokens(tokens) {}

		Module parse() {
			pos = 0;
			std::vector<Module::Import> imports;
			std::vector<Class> classes;
			while(tokens[pos].type == LexemeType::IMPORT) {
				++pos;
				bool standard = tokens[pos].type == LexemeType::NEGATION;
				if(standard) {
					++pos;
				}
				imports.emplace_back(parse_string(), standard);
			}
			while(tokens[pos].type != LexemeType::END_OF_FILE) {
				classes.push_back(parse_class());
			}
			return Module(move(imports), move(classes));
		}
	public:
		static Module parse(const std::string& module) {
			std::vector<lexer::Lexeme> tokens = tokenize(module);
			TokensView tv(tokens, module);
			return Parser(tv).parse();
		}
	};

	const std::map<LexemeType, int> Parser::priorities = {
		{LexemeType::MUL, 2}, {LexemeType::DIV, 2}, {LexemeType::MOD, 2},
		{LexemeType::ADD, 3}, {LexemeType::SUB, 3},
		{LexemeType::LESS, 4}, {LexemeType::LE, 4}, {LexemeType::MORE, 4}, {LexemeType::ME, 4},
		{LexemeType::EQ, 5}, {LexemeType::NE, 5},
		{LexemeType::AND, 6},
		{LexemeType::OR, 7}
	};

	std::string read_file(const std::filesystem::path& path) {
		std::ifstream i;
		i.exceptions (std::ifstream::failbit | std::ifstream::badbit);
		i.open(path);
		i.seekg(0, std::ios::end);
		std::ifstream::pos_type file_size = i.tellg();
		i.seekg(0, std::ios::beg);

		std::string bytes(file_size, 0);
		i.read(bytes.data(), file_size);
		return bytes;
	}
}

ParserError::ParserError(std::vector<std::filesystem::path> parsing_stack, std::optional<std::string> message, std::optional<std::tuple<size_t, size_t, std::string>> text) : parsing_stack(move(parsing_stack)), message(move(message)), text(move(text)) {}

std::vector<ParsedModule> parse_program(std::string path) {
	std::vector<ParsedModule> modules;
	std::vector<std::filesystem::path*> parsing_stack;
	std::set<std::filesystem::path> already_parsed;

	const std::filesystem::path stdlib_path = get_executable_path().parent_path() / "stdlib";

	std::function<void(std::filesystem::path)> program_parser;

	auto make_error = [&](std::string msg, std::optional<std::tuple<size_t, size_t, std::string>> text = {}){
		std::vector<std::filesystem::path> err_parsing_stack;
		err_parsing_stack.reserve(parsing_stack.size());
		for(const std::filesystem::path* s : parsing_stack) {
			err_parsing_stack.push_back(*s);
		}
		return ParserError(move(err_parsing_stack), move(msg), std::move(text));
	};

	program_parser = [&](std::filesystem::path path) {
		parsing_stack.push_back(&path);
		try {
			path = std::filesystem::canonical(path);
		} catch (const std::filesystem::filesystem_error&) {
			throw make_error("Cannot convert relative path to canonical path");
		}
		if(path.extension() != FILENAME_EXTENSION) {
			throw make_error("Filenames must have extension " + std::string(FILENAME_EXTENSION));
		}
		if(already_parsed.find(path) != already_parsed.end()) {
			parsing_stack.pop_back();
			return;
		}
		parsing_stack.pop_back();
		for(const std::filesystem::path* s : parsing_stack) {
			if(*s == path) {
				parsing_stack.push_back(&path);
				throw make_error("Module dependency cycle");
			}
		}
		parsing_stack.push_back(&path);
		std::string text;
		try {
			text = read_file(path);
		}
		catch (...) {
			throw make_error("IO error");
		}
		std::optional<Module> module_object;
		try {
			module_object = Parser::parse(text);
		} catch (const Parser::Error& e) {
			throw make_error("Syntax error", std::make_tuple(e.begin, e.end, move(text)));
		} catch (const LexerError& e) {
			throw make_error(std::move(e.msg), std::make_tuple(e.begin, e.end, move(text)));
		}
		std::filesystem::path base_path = path.parent_path();
		for(const Module::Import& i : module_object->imports) {
			program_parser(i.standard ? stdlib_path / i.path : base_path / i.path);
		}
		modules.push_back(ParsedModule(path, move(text), std::move(*module_object)));
		already_parsed.insert(path);
		parsing_stack.pop_back();
	};

	program_parser(path);

	return modules;
}

ParsedModule::ParsedModule(std::string path, std::string text, ast::Module module) : path(move(path)), text(move(text)), module(std::move(module)) {}

std::ostream& operator<<(std::ostream& o, const ParserError& e) {
	for(const std::string m : e.parsing_stack) {
		o << "In module " << m << ":\n";
	}
	if(e.message) {
		o << "Error: " << *(e.message) << "\n";
	} else {
		o << "Error.\n";
	}
	if(e.text) {
		o << position(std::get<2>(*(e.text)), std::get<0>(*(e.text)), std::get<1>(*(e.text))) <<'\n';
		print_lines(std::get<2>(*(e.text)), std::get<0>(*(e.text)), std::get<1>(*(e.text)), o, RED);
		o << '\n';
	}
	return o;
}
