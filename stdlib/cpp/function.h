#include <functional>
#include <stdexcept>

template <typename R, typename... As>
class TypeA_Function {
	std::function<std::tuple<🍆::StrongObject<R>>(🍆::StrongObject<As>&...)> fun;
public:
	🍆::WeakObject<TypeA_Function> this_ptr;
	🍆::StrongObject<TypeA_Function> get_this() {
		return 🍆::scast<TypeA_Function>(this_ptr);
	}
	virtual ~TypeA_Function() = default;

	template<typename... Ts>
	TypeA_Function(🍆::WeakObject<TypeA_Function> this_ptr, std::function<std::tuple<🍆::StrongObject<R>>(🍆::StrongObject<As>...)> f, 🍆::StrongObject<Ts>... to_capture) :
		fun(🍆::wrap_lambda([captured_args{🍆::Journal::capture(to_capture...)}, foo{std::move(f)}](🍆::StrongObject<As>&... arguments)mutable{
			auto old_arg_tags = std::apply([](auto&... a)mutable{return 🍆::Journal::get_tags(a...);}, captured_args);
			🍆::Journal::captured_get(captured_args);
			auto r = foo(arguments...);
			🍆::Journal::goto_tags(old_arg_tags, captured_args);
			return r;
		})),
		this_ptr(this_ptr) {}

	TypeA_Function(const TypeA_Function&) = delete;

	TypeA_Function(TypeA_Function&&) = delete;

	std::tuple<🍆::StrongObject<R>> basefun_call(🍆::StrongObject<As>... args) {
		return fun(args...);
	}

	virtual 🍆::StrongObject<R> privfun_call(🍆::StrongObject<As>... args) {
		return std::get<0>(basefun_call(args...));
	}
	virtual 🍆::StrongObject<R> fun_call(🍆::StrongObject<As>... args) {
		return std::get<0>(basefun_call(args...));
	}
};

template<typename R>
class Type_Function0 : public TypeA_Function<R> {
	using TypeA_Function<R>::TypeA_Function;
};

template<typename R, typename A1>
class Type_Function1 : public TypeA_Function<R, A1> {
	using TypeA_Function<R, A1>::TypeA_Function;
};

template<typename R, typename A1, typename A2>
class Type_Function2 : public TypeA_Function<R, A1, A2> {
	using TypeA_Function<R, A1, A2>::TypeA_Function;
};

template<typename R, typename A1, typename A2, typename A3>
class Type_Function3 : public TypeA_Function<R, A1, A2, A3> {
	using TypeA_Function<R, A1, A2, A3>::TypeA_Function;
};

template<typename R, typename A1, typename A2, typename A3, typename A4>
class Type_Function4 : public TypeA_Function<R, A1, A2, A3, A4> {
	using TypeA_Function<R, A1, A2, A3, A4>::TypeA_Function;
};

template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
class Type_Function5 : public TypeA_Function<R, A1, A2, A3, A4, A5> {
	using TypeA_Function<R, A1, A2, A3, A4, A5>::TypeA_Function;
};

template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
class Type_Function6 : public TypeA_Function<R, A1, A2, A3, A4, A5, A6> {
	using TypeA_Function<R, A1, A2, A3, A4, A5, A6>::TypeA_Function;
};

template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
class Type_Function7 : public TypeA_Function<R, A1, A2, A3, A4, A5, A6, A7> {
	using TypeA_Function<R, A1, A2, A3, A4, A5, A6, A7>::TypeA_Function;
};

template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
class Type_Function8 : public TypeA_Function<R, A1, A2, A3, A4, A5, A6, A7, A8> {
	using TypeA_Function<R, A1, A2, A3, A4, A5, A6, A7, A8>::TypeA_Function;
};

template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
class Type_Function9 : public TypeA_Function<R, A1, A2, A3, A4, A5, A6, A7, A8, A9> {
	using TypeA_Function<R, A1, A2, A3, A4, A5, A6, A7, A8, A9>::TypeA_Function;
};

#define 🍆_Default_Function0
#define 🍆_Default_Function1
#define 🍆_Default_Function2
#define 🍆_Default_Function3
#define 🍆_Default_Function4
#define 🍆_Default_Function5
#define 🍆_Default_Function6
#define 🍆_Default_Function7
#define 🍆_Default_Function8
#define 🍆_Default_Function9

