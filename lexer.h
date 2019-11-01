#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <tuple>
#include <string>

namespace lexer {

	struct LexerError : std::exception {
		const size_t begin;
		const size_t end;
		const char* msg;
		const char* what() const noexcept;
		LexerError() = delete;
		LexerError(size_t begin, size_t end, const char* msg);
	};

	enum class LexemeType {
		SNAPSHOT, AUTO, THIS, MOD, NIL, CAST, NEW, ELSE, IF, WHILE, RETURN, VOID, STRONG, WEAK, END_OF_FILE, OPTIONAL, BREAK, CONTINUE, FOR, AND, OR, DOT, COMMA, IDENTIFIER, VAR, FUN, INT, LESS, LE, EQ, NE, MORE, ME, ABSTRACT, NEGATION, ASSIGNMENT, NOJOURNAL, STRUCT, BRACKET_OPEN, BRACKET_CLOSE, CURLY_OPEN, CURLY_CLOSE, SQUARE_OPEN, SQUARE_CLOSE, CONSTRUCTOR, DESTRUCTOR, IMPORT, NOEFFECT, DUAL, COLON, SEMICOLON, DIV, MUL, ADD, SUB, STRING_LITERAL, INT_LITERAL, BYTE_LITERAL
	};

	struct Lexeme {
		const size_t begin;
		const size_t end;
		const LexemeType type;
		Lexeme() = delete;
		Lexeme(size_t begin, size_t end, LexemeType type);
	};

	std::vector<Lexeme> tokenize(const std::string& program);

}

#endif
