#ifndef TYPECHECKER_H
#define TYPECHECKER_H

#include <vector>
#include <optional>

#include "ast.h"
#include "parser.h"
#include "util.h"

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
	static void print_errors(std::ostream& o, const std::vector<ParsedModule>& modules, const std::pair<std::vector<TypeError>, std::vector<TypeError>>& errors);
};

std::pair<std::vector<TypeError>, std::vector<TypeError>> typecheck(const std::vector<ParsedModule>&);

#endif
