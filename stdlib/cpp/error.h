#include <exception>

#define 🍆_Default_Error \
struct Type_Error { \
	🍆::StrongObject<🍆::Integer> var_something; \
	🍆::WeakObject<Type_Error> this_ptr; \
	🍆::StrongObject<Type_Error> get_this() { \
		return 🍆::scast<Type_Error>(this_ptr); \
	} \
	virtual ~Type_Error() = default; \
	Type_Error(WeakObject<Type_Error> this_ptr, 🍆::StrongObject<Type_String> var_message) : this_ptr(this_ptr) { \
		🍆::StrongObject<Type_StdErr> var_s = (🍆::make_object<Type_StdErr>()); \
		((var_s)->fun_print_string((var_message))); \
		var_s->fun_print_newline(); \
		((var_s)->fun_flush()); \
		std::terminate(); \
	} \
	Type_Error() = delete; \
	Type_Error(const Type_Error&) = delete; \
	Type_Error(Type_Error&&) = delete; \
};
