#include <vector>

template<typename T>
class Type_Array {
public:
	🍆::WeakObject<Type_Array> this_ptr;
	🍆::StrongObject<Type_Array<T>> get_this() {
		return 🍆::scast<Type_Array<T>>(this_ptr);
	}

	std::vector<StrongObject<T>> vec;

	virtual ~Type_Array() = default;

	Type_Array(WeakObject<Type_Array<T>> this_ptr) : this_ptr(this_ptr) {
	}

	Type_Array(const Type_Array&) = delete;

	Type_Array(Type_Array&&) = delete;

	virtual 🍆::StrongObject<T> fun_at(🍆::StrongObject<🍆::Integer> var_pos) {
		return vec[var_pos->to_ll()];
	}
	virtual 🍆::StrongObject<T> fun_pop_back() {
		🍆::StrongObject<T> el = vec.back();
		vec.pop_back();
		return el;
	}
	virtual void fun_push_back(🍆::StrongObject<T> var_element) {
		vec.push_back(std::move(var_element));
	}
	virtual 🍆::StrongObject<T> fun_set(🍆::StrongObject<🍆::Integer> var_pos, 🍆::StrongObject<T> var_element) {
		auto pos = var_pos->to_ll();
		🍆::StrongObject<T> old = vec[pos];
		vec[pos] = var_element;
		return old;
	}
	virtual 🍆::StrongObject<🍆::Integer> fun_size() {
		return 🍆::StrongObject<🍆::Integer>(vec.size());
	}
};

#define 🍆_Default_Array
