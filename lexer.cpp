#include "lexer.h"

#include <map>

namespace lexer {

	LexerError::LexerError(size_t begin, size_t end, const char* msg) : begin(begin), end(end), msg(msg) {}

	const char* LexerError::what() const noexcept {
		return msg;
	}

	Lexeme::Lexeme(size_t begin, size_t end, LexemeType type) : begin(begin), end(end), type(type) {}

	std::vector<Lexeme> tokenize(const std::string& program) {
		std::vector<Lexeme> tokens;
		size_t pos = 0;
		static const auto is_whitespace = [](char i){
			switch(i) {
			case ' ':
				return true;
			case '\t':
				return true;
			case '\n':
				return true;
			default:
				return false;
			}
		};
		auto skip_whitespaces = [&]() {
			while(true) {
				size_t oldpos = pos;
				while(pos < program.size() && is_whitespace(program[pos])) {
					++pos;
				}
				if(pos + 1 < program.size() && program[pos] == '/' && program[pos + 1] == '*') {
					pos += 2;
					bool found = false;
					while(pos < program.size()) {
						if(program[pos] == '\\') {
							++pos;
							continue;
						}
						if(program[pos] == '*' && pos + 1 < program.size() && program[pos + 1] == '/') {
							pos += 2;
							found = true;
							break;
						}
						++pos;
					}
					if(!found) {
						throw LexerError(oldpos, pos, "Comment doesn't end.");
					}
				}
				if(oldpos == pos) {
					break;
				}
			}
		};
		auto try_read = [&](const std::string& text, LexemeType type) {
			if(program.size() - pos < text.size()) {
				return false;
			}
			for(size_t i = 0; i < text.size(); ++i) {
				if(program[pos + i] != text[i]) {
					return false;
				}
			}
			tokens.push_back(Lexeme(pos, pos + text.size(), type));
			pos += text.size();
			return true;
		};
		static const std::map<std::string, LexemeType> keywords{{"this", LexemeType::THIS}, {"null", LexemeType::NIL}, {"cast", LexemeType::CAST}, {"new", LexemeType::NEW}, {"else", LexemeType::ELSE}, {"if", LexemeType::IF}, {"while", LexemeType::WHILE}, {"return", LexemeType::RETURN}, {"void", LexemeType::VOID}, {"strong", LexemeType::STRONG}, {"weak", LexemeType::WEAK}, {"optional", LexemeType::OPTIONAL}, {"continue", LexemeType::CONTINUE}, {"break", LexemeType::BREAK}, {"for", LexemeType::FOR}, {"var", LexemeType::VAR}, {"fun", LexemeType::FUN}, {"int", LexemeType::INT}, {"abstract", LexemeType::ABSTRACT}, {"nojournal", LexemeType::NOJOURNAL}, {"struct", LexemeType::STRUCT}, {"constructor", LexemeType::CONSTRUCTOR}, {"destructor", LexemeType::DESTRUCTOR}, {"import", LexemeType::IMPORT}, {"noeffect", LexemeType::NOEFFECT}, {"dual", LexemeType::DUAL}, {"irreversible", LexemeType::IRREVERSIBLE}};
		static const std::vector<std::pair<std::string, LexemeType>> operators{{"%", LexemeType::MOD}, {"&&", LexemeType::AND}, {"||", LexemeType::OR}, {".", LexemeType::DOT}, {",", LexemeType::COMMA}, {"<=", LexemeType::LE}, {"<", LexemeType::LESS}, {">=", LexemeType::ME}, {">", LexemeType::MORE}, {"==", LexemeType::EQ}, {"!=", LexemeType::NE}, {"!", LexemeType::NEGATION}, {"=", LexemeType::ASSIGNMENT}, {"(", LexemeType::BRACKET_OPEN}, {")", LexemeType::BRACKET_CLOSE}, {"{", LexemeType::CURLY_OPEN}, {"}", LexemeType::CURLY_CLOSE}, {"[", LexemeType::SQUARE_OPEN}, {"]", LexemeType::SQUARE_CLOSE}, {":", LexemeType::COLON}, {";", LexemeType::SEMICOLON}, {"/", LexemeType::DIV}, {"*", LexemeType::MUL}, {"+", LexemeType::ADD}, {"-", LexemeType::SUB}};
		auto try_read_int_literal = [&](){
			size_t oldpos = pos;
			while(pos < program.size() && program[pos] >= '0' && program[pos] <= '9') {
				++pos;
			}
			if(oldpos != pos) {
				tokens.push_back(Lexeme(oldpos, pos, LexemeType::INT_LITERAL));
				return true;
			} else {
				return false;
			}
		};
		auto try_read_byte_literal = [&](){
			if(pos + 2 < program.size() && program[pos] == '\'' && program[pos + 2] == '\'') {
				pos += 3;
				tokens.push_back(Lexeme(pos - 3, pos, LexemeType::BYTE_LITERAL));
				return true;
			}
			if(pos + 3 < program.size() && program[pos] == '\'' && program[pos + 1] == '\\' && program[pos + 3] == '\'') {
				pos += 4;
				tokens.push_back(Lexeme(pos - 4, pos, LexemeType::BYTE_LITERAL));
				return true;
			}
			return false;
		};
		auto try_read_string_literal = [&](){
			if(pos >= program.size() || program[pos] != '"') {
				return false;
			}
			size_t oldpos = pos++;
			while(pos < program.size() && program[pos] != '"') {
				if(program[pos] == '\\') {
					++pos;
				}
				++pos;
			}
			if(pos < program.size() && program[pos] == '"') {
				++pos;
				tokens.push_back(Lexeme(oldpos, pos, LexemeType::STRING_LITERAL));
				return true;
			} else {
				pos = oldpos;
				return false;
			}
		};
		auto try_read_identifier_or_keyword = [&](){
			if(pos >= program.size()) {
				return false;
			}
			if(program[pos] == '_') {
				throw LexerError(pos, pos + 1, "Identifier begins with an underscore.");
			}
			size_t oldpos = pos;
			bool last_underscore = false;
			while(pos < program.size()) {
				if(program[pos] >= 'a' && program[pos] <= 'z') {
					++pos;
					continue;
				}
				if(program[pos] >= 'A' && program[pos] <= 'Z') {
					++pos;
					continue;
				}
				if(program[pos] >= '0' && program[pos] <= '9') {
					++pos;
					continue;
				}
				if(program[pos] == '_') {
					if(last_underscore) {
						throw LexerError(oldpos, pos + 1, "Two consecutive underscores in an indentifier.");
					}
					last_underscore = true;
					++pos;
					continue;
				}
				break;
			}
			if(pos != oldpos) {
				auto t = keywords.find(program.substr(oldpos, pos - oldpos));
				if(t != keywords.end()) {
					tokens.push_back(Lexeme(oldpos, pos, t->second));
				} else {
					tokens.push_back(Lexeme(oldpos, pos, LexemeType::IDENTIFIER));
				}
				return true;
			} else {
				pos = oldpos;
				return false;
			}
		};
		while(true) {
			skip_whitespaces();
			bool read = false;
			for(const auto&[text, type] : operators){
				if(try_read(text, type)) {
					read = true;
					break;
				}
			}
			if(read) {
				continue;
			}
			if(try_read_string_literal()) {
				continue;
			}
			if(try_read_int_literal()) {
				continue;
			}
			if(try_read_byte_literal()) {
				continue;
			}
			if(try_read_identifier_or_keyword()) {
				continue;
			}
			if(pos != program.size()) {
				size_t f = pos + 1;
				while(f < program.size() && !is_whitespace(program[f])) {
					++f;
				}
				throw LexerError(pos, f, "Unrecognized tokens.");
			}
			break;
		}
		return tokens;
	}

}
