#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"

#include <tuple>
#include <filesystem>

struct ParsedModule {
	std::string path;
	std::string text;
	ast::Module module;
	ParsedModule() = delete;
	ParsedModule(std::string path, std::string text, ast::Module module);
};

std::vector<ParsedModule> parse_program(std::string path);

struct ParserError : std::exception {
	const std::vector<std::filesystem::path> parsing_stack;
	const std::optional<std::string> message;
	const std::optional<std::tuple<size_t, size_t, std::string>> text;
	ParserError() = delete;
private:
	ParserError(std::vector<std::filesystem::path> parsing_stack, std::optional<std::string> message, std::optional<std::tuple<size_t, size_t, std::string>> text);
	friend std::vector<ParsedModule> parse_program(std::string path);
};

std::ostream& operator<<(std::ostream& o, const ParserError& e);

#endif
