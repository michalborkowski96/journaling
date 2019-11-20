#include <random>
#include <limits>

#define 🍆_Default_RandInt
struct Type_RandInt {
	static std::random_device rd;
	static std::mt19937_64 eng;

	🍆::StrongObject<🍆::Integer> var_result;
	🍆::WeakObject<Type_RandInt> this_ptr;
	🍆::StrongObject<Type_RandInt> get_this() {
		return 🍆::scast<Type_RandInt>(this_ptr);
	}
	virtual ~Type_RandInt() = default;
	Type_RandInt(WeakObject<Type_RandInt> this_ptr) : this_ptr(this_ptr) {
		static std::uniform_int_distribution<long long> distr(std::numeric_limits<long long>::min());
		var_result = distr(eng);
	}
	Type_RandInt(WeakObject<Type_RandInt> this_ptr, 🍆::StrongObject<🍆::Integer> var_min, 🍆::StrongObject<🍆::Integer> var_max) : this_ptr(this_ptr) {
		std::uniform_int_distribution<long long> distr(var_min.get_value(), var_max.get_value());
		var_result = distr(eng);
	}
	Type_RandInt(const Type_RandInt&) = delete;
	Type_RandInt(Type_RandInt&&) = delete;
};

std::random_device Type_RandInt::rd = std::random_device();
std::mt19937_64 Type_RandInt::eng(Type_RandInt::rd());
