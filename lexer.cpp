#include "lexer.h"

#include <map>

namespace lexer {

	const std::map<std::string, std::string> allowed_unicode_chars = {
	{u8"\u00A8", u8"\u00A8"},
	{u8"\u00AA", u8"\u00AA"},
	{u8"\u00AD", u8"\u00AD"},
	{u8"\u00AF", u8"\u00AF"},
	{u8"\u00B2", u8"\u00B5"},
	{u8"\u00B7", u8"\u00BA"},
	{u8"\u00BC", u8"\u00BE"},
	{u8"\u00C0", u8"\u00D6"},
	{u8"\u00D8", u8"\u00F6"},
	{u8"\u00F8", u8"\u07FF"},
	{u8"\u0800", u8"\u167F"},
	{u8"\u1681", u8"\u180D"},
	{u8"\u180F", u8"\u1FFF"},
	{u8"\u200B", u8"\u200D"},
	{u8"\u202A", u8"\u202E"},
	{u8"\u203F", u8"\u2040"},
	{u8"\u2054", u8"\u2054"},
	{u8"\u2060", u8"\u218F"},
	{u8"\u2460", u8"\u24FF"},
	{u8"\u2776", u8"\u2793"},
	{u8"\u2C00", u8"\u2DFF"},
	{u8"\u2E80", u8"\u2FFF"},
	{u8"\u3004", u8"\u3007"},
	{u8"\u3021", u8"\u302F"},
	{u8"\u3031", u8"\uD7FF"},
	{u8"\uF900", u8"\uFD3D"},
	{u8"\uFD40", u8"\uFDCF"},
	{u8"\uFDF0", u8"\uFE44"},
	{u8"\uFE47", u8"\uFFFD"},
	{u8"\U00010000", u8"\U0001FFFD"},
	{u8"\U00020000", u8"\U0002FFFD"},
	{u8"\U00030000", u8"\U0003FFFD"},
	{u8"\U00040000", u8"\U0004FFFD"},
	{u8"\U00050000", u8"\U0005FFFD"},
	{u8"\U00060000", u8"\U0006FFFD"},
	{u8"\U00070000", u8"\U0007FFFD"},
	{u8"\U00080000", u8"\U0008FFFD"},
	{u8"\U00090000", u8"\U0009FFFD"},
	{u8"\U000A0000", u8"\U000AFFFD"},
	{u8"\U000B0000", u8"\U000BFFFD"},
	{u8"\U000C0000", u8"\U000CFFFD"},
	{u8"\U000D0000", u8"\U000DFFFD"},
	{u8"\U000E0000", u8"\U000EFFFD"}
	};

	const std::map<std::string, std::string> not_beginning_unicode_chars = {
	{u8"\u0300", u8"\u036F"},
	{u8"\u1DC0", u8"\u1DFF"},
	{u8"\u20D0", u8"\u20FF"},
	{u8"\uFE20", u8"\uFE2F"}
	};

	bool in_unicode_ranges(const std::string& c, const std::map<std::string, std::string>& ranges) {
		auto i = ranges.upper_bound(c);
		if(i == ranges.begin()) {
			return false;
		}
		--i;
		return c >= i->first && c <= i->second;
	}

	LexerError::LexerError(size_t begin, size_t end, const char* msg) : begin(begin), end(end), msg(msg) {}

	const char* LexerError::what() const noexcept {
		return msg;
	}

	size_t get_unicode_char_len(const char* text) {
		auto at = [&](size_t i){
			return ((const uint8_t*) text)[i];
		};
		if((at(0) >> 5) == 6U && (at(1) >> 6) == 2U) {
			return 2;
		}
		if((at(0) >> 4) == 14U && (at(1) >> 6) == 2U && (at(2) >> 6) == 2U) {
			return 3;
		}
		if((at(0) >> 3) == 30U && (at(1) >> 6) == 2U && (at(2) >> 6) == 2U && (at(3) >> 6) == 2U) {
			return 4;
		}
		return 0;
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
		static const std::map<std::string, LexemeType> keywords{{"snapshot", LexemeType::SNAPSHOT}, {"this", LexemeType::THIS}, {"auto", LexemeType::AUTO}, {"null", LexemeType::NIL}, {"cast", LexemeType::CAST}, {"new", LexemeType::NEW}, {"else", LexemeType::ELSE}, {"if", LexemeType::IF}, {"while", LexemeType::WHILE}, {"return", LexemeType::RETURN}, {"void", LexemeType::VOID}, {"strong", LexemeType::STRONG}, {"weak", LexemeType::WEAK}, {"optional", LexemeType::OPTIONAL}, {"continue", LexemeType::CONTINUE}, {"break", LexemeType::BREAK}, {"for", LexemeType::FOR}, {"var", LexemeType::VAR}, {"fun", LexemeType::FUN}, {"int", LexemeType::INT}, {"abstract", LexemeType::ABSTRACT}, {"nojournal", LexemeType::NOJOURNAL}, {"struct", LexemeType::STRUCT}, {"constructor", LexemeType::CONSTRUCTOR}, {"destructor", LexemeType::DESTRUCTOR}, {"import", LexemeType::IMPORT}, {"noeffect", LexemeType::NOEFFECT}, {"dual", LexemeType::DUAL}};
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
			{
				size_t c = get_unicode_char_len(program.data() + pos);
				if(c) {
					if(in_unicode_ranges(program.substr(pos, c), not_beginning_unicode_chars)) {
						throw LexerError(pos, pos + c, "Identifier begins with an invalid character.");
					}
				}
			}
			size_t oldpos = pos;
			bool last_underscore = false;
			while(pos < program.size()) {
				if(program[pos] >= 'a' && program[pos] <= 'z') {
					++pos;
					last_underscore = false;
					continue;
				}
				if(program[pos] >= 'A' && program[pos] <= 'Z') {
					++pos;
					last_underscore = false;
					continue;
				}
				if(program[pos] >= '0' && program[pos] <= '9') {
					++pos;
					last_underscore = false;
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
				size_t c = get_unicode_char_len(program.data() + pos);
				if(c) {
					if(in_unicode_ranges(program.substr(pos, c), allowed_unicode_chars)) {
						last_underscore = false;
						pos += c;
					}
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
