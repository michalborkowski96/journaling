#include "util.h"

void print_lines(const std::string& program, size_t begin, size_t end, std::ostream& out, const char* colour) {
	int pre = 1;
	int post = 2;

	size_t b = begin;
	while(b && pre != -1) {
		--b;
		if(program[b] == '\n') {
			--pre;
		}
	}
	if(b && b != begin) {
		++b;
	}

	size_t e = end;
	while(e < program.size() && post != -1) {
		++e;
		if(program[e] == '\n') {
			--post;
		}
	}

	for(size_t i = b; i < end; ++i) {
		if(i == begin) {
		out << colour;
		}
		out << program[i];
	}
	out << ENDCOLOUR;
	for(size_t i = end; i < e; ++i) {
		out << program[i];
	}
}

std::pair<size_t, size_t> position(const std::string& program, size_t pos) {
	size_t line = 1;
	size_t line_pos = 0;
	for(size_t i = 0; i <= pos && i < program.size(); ++i) {
		if(program[i] == '\n') {
			line_pos = i;
			++line;
		}
	}
	return std::make_pair(line, pos - line_pos);
}

std::string position(const std::string& program, size_t begin, size_t end) {
	std::pair<size_t, size_t> b = position(program, begin);
	std::pair<size_t, size_t> e = position(program, end);
	auto s = [](const std::pair<size_t, size_t> f) {
		return "line " + std::to_string(f.first) + " column " + std::to_string(f.second);
	};
	return "from " + s(b) + " to " + s(e);
}
