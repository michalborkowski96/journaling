#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
#include <ostream>

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

constexpr const char* const ENDCOLOUR = "\033[m";

#endif
