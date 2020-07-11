#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <ostream>

constexpr const char* const ADD_FUN_NAME = "add";
constexpr const char* const SUB_FUN_NAME = "sub";
constexpr const char* const MUL_FUN_NAME = "mul";
constexpr const char* const DIV_FUN_NAME = "div";
constexpr const char* const OR_FUN_NAME = "alt";
constexpr const char* const AND_FUN_NAME = "con";
constexpr const char* const NEG_FUN_NAME = "neg";
constexpr const char* const OPP_FUN_NAME = "opp";
constexpr const char* const MOD_FUN_NAME = "mod";
constexpr const char* const LESS_FUN_NAME = "less";
constexpr const char* const LESS_EQUAL_FUN_NAME = "le";
constexpr const char* const MORE_FUN_NAME = "more";
constexpr const char* const MORE_EQUAL_FUN_NAME = "me";
constexpr const char* const EQUAL_FUN_NAME = "eq";
constexpr const char* const NOT_EQUAL_FUN_NAME = "neq";

constexpr const char* const STRING_TYPE_NAME = "String";
constexpr const char* const INT_TYPE_NAME = "int";
constexpr const char* const NULL_TYPE_NAME = "null";
constexpr const char* const VOID_TYPE_NAME = "void";

constexpr const char* const THIS_NAME = "this";

constexpr const char* const FILENAME_EXTENSION = ".🍆";

using Integer = int64_t;

template<template <typename...> typename T, template <typename, typename...> typename U, typename... Vs>
using Wrap = T<U<Vs>...>;

template <typename... Fs>
struct overload : std::remove_reference_t<Fs>... {
	overload(Fs... fs) : std::remove_reference_t<Fs>{std::forward<Fs>(fs)}...{}
	using std::remove_reference_t<Fs>::operator()...;
};

void print_lines(const std::string& program, size_t begin, size_t end, std::ostream& out, const char* colour);

std::pair<size_t, size_t> position(const std::string& program, size_t pos);

std::string position(const std::string& program, size_t begin, size_t end);

constexpr const char* const RED = "\033[1;31m";

constexpr const char* const YELLOW = "\033[1;33m";

constexpr const char* const GREEN = "\033[1;32m";

constexpr const char* const ENDCOLOUR = "\033[m";

template<size_t N = 0, typename F, typename... Ts>
void for_each_tuple(F&& function, std::tuple<Ts...>& tuple) {
	if constexpr (N < std::tuple_size<std::tuple<Ts...>>::value) {
		function(std::get<N>(tuple));
		for_each_tuple<N + 1>(std::forward<F>(function), tuple);
	}
}

template<size_t N = 0, typename F, typename... Ts>
void for_each_tuple(F&& function, const std::tuple<Ts...>& tuple) {
	if constexpr (N < std::tuple_size<std::tuple<Ts...>>::value) {
		function(std::get<N>(tuple));
		for_each_tuple<N + 1>(std::forward<F>(function), tuple);
	}
}

#endif
