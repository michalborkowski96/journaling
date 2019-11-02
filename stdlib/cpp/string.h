#include <string>

#define 🍆_Default_String \
class Type_String { \
public: \
	std::string data; \
	🍆::WeakObject<Type_String> this_ptr; \
	virtual ~Type_String() = default; \
	Type_String(WeakObject<Type_String> this_ptr) : this_ptr(this_ptr) { \
	} \
	Type_String(WeakObject<Type_String> this_ptr, 🍆::StrongObject<🍆::Integer> var_char_ptr, 🍆::StrongObject<🍆::Integer> var_length) : this_ptr(this_ptr), data((char*) (var_char_ptr->to_ll()), var_length->to_ll()) {} \
	Type_String(const Type_String&) = delete; \
	Type_String(Type_String&&) = delete; \
	Type_String(WeakObject<Type_String> this_ptr, std::string&& data) : this_ptr(this_ptr), data(std::move(data)) {} \
	auto basefun_add(🍆::StrongObject<Type_String> var_s) { \
		return std::make_tuple(🍆::make_object<Type_String>(data + var_s->data)); \
	} \
	virtual 🍆::StrongObject<Type_String> privfun_add(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_add(var_s)); \
	} \
	virtual 🍆::StrongObject<Type_String> fun_add(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_add(var_s)); \
	} \
	auto basefun_append(🍆::StrongObject<Type_String> var_s) { \
		data += var_s->data; \
	} \
	virtual void privfun_append(🍆::StrongObject<Type_String> var_s) { \
		(basefun_append(var_s)); \
	} \
	virtual void fun_append(🍆::StrongObject<Type_String> var_s) { \
		(basefun_append(var_s)); \
	} \
	auto basefun_clear() { \
		data.clear(); \
	} \
	virtual void privfun_clear() { \
		(basefun_clear()); \
	} \
	virtual void fun_clear() { \
		(basefun_clear()); \
	} \
	auto basefun_empty() { \
		return std::make_tuple(🍆::StrongObject<Integer>(data.empty())); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> privfun_empty() { \
		return std::get<0>(basefun_empty()); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> fun_empty() { \
		return std::get<0>(basefun_empty()); \
	} \
	auto basefun_eq(🍆::StrongObject<Type_String> var_s) { \
		return std::make_tuple(🍆::StrongObject<Integer>(data == var_s->data)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> privfun_eq(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_eq(var_s)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> fun_eq(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_eq(var_s)); \
	} \
	auto basefun_get(🍆::StrongObject<🍆::Integer> var_i) { \
		return std::make_tuple(🍆::StrongObject<Integer>(data[var_i->to_ll()])); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> privfun_get(🍆::StrongObject<🍆::Integer> var_i) { \
		return std::get<0>(basefun_get(var_i)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> fun_get(🍆::StrongObject<🍆::Integer> var_i) { \
		return std::get<0>(basefun_get(var_i)); \
	} \
	auto basefun_le(🍆::StrongObject<Type_String> var_s) { \
		return std::make_tuple(🍆::StrongObject<Integer>(data <= var_s->data)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> privfun_le(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_le(var_s)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> fun_le(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_le(var_s)); \
	} \
	auto basefun_less(🍆::StrongObject<Type_String> var_s) { \
		return std::make_tuple(🍆::StrongObject<Integer>(data < var_s->data)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> privfun_less(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_less(var_s)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> fun_less(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_less(var_s)); \
	} \
	auto basefun_me(🍆::StrongObject<Type_String> var_s) { \
		return std::make_tuple(🍆::StrongObject<Integer>(data >= var_s->data)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> privfun_me(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_me(var_s)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> fun_me(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_me(var_s)); \
	} \
	auto basefun_more(🍆::StrongObject<Type_String> var_s) { \
		return std::make_tuple(🍆::StrongObject<Integer>(data > var_s->data)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> privfun_more(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_more(var_s)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> fun_more(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_more(var_s)); \
	} \
	auto basefun_neq(🍆::StrongObject<Type_String> var_s) { \
		return std::make_tuple(🍆::StrongObject<Integer>(data != var_s->data)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> privfun_neq(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_neq(var_s)); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> fun_neq(🍆::StrongObject<Type_String> var_s) { \
		return std::get<0>(basefun_neq(var_s)); \
	} \
	auto basefun_resize(🍆::StrongObject<🍆::Integer> var_new_size, 🍆::StrongObject<🍆::Integer> var_char) { \
		data.resize(var_new_size->to_ll(), var_char->to_ll()); \
	} \
	virtual void privfun_resize(🍆::StrongObject<🍆::Integer> var_new_size, 🍆::StrongObject<🍆::Integer> var_char) { \
		(basefun_resize(var_new_size, var_char)); \
	} \
	virtual void fun_resize(🍆::StrongObject<🍆::Integer> var_new_size, 🍆::StrongObject<🍆::Integer> var_char) { \
		(basefun_resize(var_new_size, var_char)); \
	} \
	auto basefun_set(🍆::StrongObject<🍆::Integer> var_i, 🍆::StrongObject<🍆::Integer> var_char) { \
		data[var_i->to_ll()] = var_char->to_ll(); \
	} \
	virtual void privfun_set(🍆::StrongObject<🍆::Integer> var_i, 🍆::StrongObject<🍆::Integer> var_char) { \
		(basefun_set(var_i, var_char)); \
	} \
	virtual void fun_set(🍆::StrongObject<🍆::Integer> var_i, 🍆::StrongObject<🍆::Integer> var_char) { \
		(basefun_set(var_i, var_char)); \
	} \
	auto basefun_size() { \
		return std::make_tuple(🍆::StrongObject<Integer>(data.size())); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> privfun_size() { \
		return std::get<0>(basefun_size()); \
	} \
	virtual 🍆::StrongObject<🍆::Integer> fun_size() { \
		return std::get<0>(basefun_size()); \
	} \
	auto basefun_substr(🍆::StrongObject<🍆::Integer> var_pos, 🍆::StrongObject<🍆::Integer> var_len) { \
		return std::make_tuple(🍆::make_object<Type_String>(data.substr(var_pos->to_ll(), var_len->to_ll()))); \
	} \
	virtual void privfun_substr(🍆::StrongObject<🍆::Integer> var_pos, 🍆::StrongObject<🍆::Integer> var_len) { \
		(basefun_substr(var_pos, var_len)); \
	} \
	virtual void fun_substr(🍆::StrongObject<🍆::Integer> var_pos, 🍆::StrongObject<🍆::Integer> var_len) { \
		(basefun_substr(var_pos, var_len)); \
	} \
};
