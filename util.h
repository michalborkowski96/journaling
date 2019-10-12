#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <cstddef>
#include <vector>

using Integer = int64_t;

template<template <typename...> typename T, template <typename, typename...> typename U, typename... Vs>
using Wrap = T<U<Vs>...>;

template <typename... Fs>
struct overload : std::remove_reference_t<Fs>... {
	overload(Fs... fs) : std::remove_reference_t<Fs>{std::forward<Fs>(fs)}...{}

	using std::remove_reference_t<Fs>::operator()...;
};

#endif
