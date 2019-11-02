#include <iostream>

#define 🍆_Default_StdOut \
class Type_StdOut : public Type_OutputStream { \
public: \
	using Type_OutputStream::this_ptr; \
	virtual ~Type_StdOut() = default; \
	Type_StdOut(WeakObject<Type_StdOut> this_ptr) : Type_OutputStream(this_ptr) { \
	} \
	Type_StdOut(const Type_StdOut&) = delete; \
	Type_StdOut(Type_StdOut&&) = delete; \
	auto basefun_flush() { \
		std::cout.flush(); \
	} \
	virtual void privfun_flush() { \
		(basefun_flush()); \
	} \
	virtual void fun_flush() { \
		(basefun_flush()); \
	} \
	auto basefun_print_string(🍆::StrongObject<Type_String> var_s) { \
		std::cout << var_s->data; \
	} \
	virtual void privfun_print_string(🍆::StrongObject<Type_String> var_s) { \
		(basefun_print_string(var_s)); \
	} \
	virtual void fun_print_string(🍆::StrongObject<Type_String> var_s) { \
		(basefun_print_string(var_s)); \
	} \
};

#define 🍆_Default_StdErr \
class Type_StdErr : public Type_OutputStream { \
public: \
	using Type_OutputStream::this_ptr; \
	virtual ~Type_StdErr() = default; \
	Type_StdErr(WeakObject<Type_StdErr> this_ptr) : Type_OutputStream(this_ptr) { \
	} \
	Type_StdErr(const Type_StdErr&) = delete; \
	Type_StdErr(Type_StdErr&&) = delete; \
	auto basefun_flush() { \
		std::cerr.flush(); \
	} \
	virtual void privfun_flush() { \
		(basefun_flush()); \
	} \
	virtual void fun_flush() { \
		(basefun_flush()); \
	} \
	auto basefun_print_string(🍆::StrongObject<Type_String> var_s) { \
		std::cerr << var_s->data; \
	} \
	virtual void privfun_print_string(🍆::StrongObject<Type_String> var_s) { \
		(basefun_print_string(var_s)); \
	} \
	virtual void fun_print_string(🍆::StrongObject<Type_String> var_s) { \
		(basefun_print_string(var_s)); \
	} \
};

#define 🍆_Default_StdIn \
class Type_StdIn : public Type_InputStream { \
public: \
	using Type_InputStream::this_ptr; \
	virtual ~Type_StdIn() = default; \
	Type_StdIn(WeakObject<Type_StdIn> this_ptr) : Type_InputStream(this_ptr) { \
	} \
	Type_StdIn(const Type_StdIn&) = delete; \
	Type_StdIn(Type_StdIn&&) = delete; \
	auto basefun_get_char() { \
		return std::make_tuple(🍆::StrongObject<🍆::Integer>(std::cin.get())); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> privfun_get_char() { \
		return std::get<0>(basefun_get_char()); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> fun_get_char() { \
		return std::get<0>(basefun_get_char()); \
	} \
	auto basefun_get_int() { \
		long long i; \
		std::cin >> i; \
		return std::make_tuple(🍆::StrongObject<🍆::Integer>(i)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> privfun_get_int() { \
		return std::get<0>(basefun_get_int()); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> fun_get_int() { \
		return std::get<0>(basefun_get_int()); \
	} \
	auto basefun_get_string() { \
		std::string str; \
		std::cin >> str; \
		return std::make_tuple(🍆::make_object<Type_String>(move(str))); \
	} \
	virtual 🍆::StrongObject<Type_String> privfun_get_string() { \
		return std::get<0>(basefun_get_string()); \
	} \
	virtual 🍆::StrongObject<Type_String> fun_get_string() { \
		return std::get<0>(basefun_get_string()); \
	} \
};
