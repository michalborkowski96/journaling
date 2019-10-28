#include <🍆>

class Type_String;

template <typename, typename>
struct Type_ListElement_;

template <typename, typename>
class Type_SimpleList_;

class Type_OutputStream;

class Type_InputStream;

class Type_StdOut;

class Type_StdErr;

class Type_StdIn;

class Type_FileIn;

class Type_FileOut;

struct Type_Error;

#ifndef 🍆String
#define 🍆String
class Type_String {
public:
	🍆::WeakObject<Type_String> _this;
	Type_String() {
	}
	
	Type_String(🍆::Integer var_char_ptr) {
	}
	
	Type_String(const Type_String&) = delete;
	
	Type_String(Type_String&&) = delete;
	
};
#endif

template <>
struct 🍆::has_journal<Type_String> : public 🍆::false_type {};

template <>
struct Type_ListElement_<🍆::Integer, 🍆::Integer> {
	🍆::StrongObject<🍆::Integer> var_element_;
	🍆::StrongObject<Type_ListElement_> var_next_;
	🍆::WeakObject<Type_ListElement_> var_prev_;
	🍆::WeakObject<Type_ListElement_> _this;
	Type_ListElement_(🍆::StrongObject<🍆::Integer> var_e, 🍆::StrongObject<Type_ListElement_> var_n, 🍆::StrongObject<Type_ListElement_> var_p) {
		(var_element_) = (var_e);
		(var_next_) = (var_n);
		(var_prev_) = (var_p);
	}
	
	Type_ListElement_() = delete;
	
	Type_ListElement_(const Type_ListElement_&) = delete;
	
	Type_ListElement_(Type_ListElement_&&) = delete;
	
};

template <>
struct 🍆::has_journal<Type_ListElement_<🍆::Integer, 🍆::Integer>> : public 🍆::false_type {};

template <>
class Type_SimpleList_<🍆::Integer, 🍆::Integer> {
	🍆::Integer var_size_;
	🍆::StrongObject<Type_ListElement_<🍆::Integer, 🍆::Integer>> var_begin_;
	🍆::StrongObject<Type_ListElement_<🍆::Integer, 🍆::Integer>> var_end_;
public:
	🍆::WeakObject<Type_SimpleList_> _this;
	Type_SimpleList_() {
		(var_end_) = (🍆::make_object<Type_ListElement_<🍆::Integer, 🍆::Integer>>((🍆::null), (🍆::null), (🍆::null)));
		(var_begin_) = (var_end_);
	}
	
	Type_SimpleList_(const Type_SimpleList_&) = delete;
	
	Type_SimpleList_(Type_SimpleList_&&) = delete;
	
};

template <>
struct 🍆::has_journal<Type_SimpleList_<🍆::Integer, 🍆::Integer>> : public 🍆::false_type {};

#ifndef 🍆OutputStream
#define 🍆OutputStream
class Type_OutputStream {
public:
	🍆::WeakObject<Type_OutputStream> _this;
	Type_OutputStream() {
	}
	
	Type_OutputStream(const Type_OutputStream&) = delete;
	
	Type_OutputStream(Type_OutputStream&&) = delete;
	
};
#endif

template <>
struct 🍆::has_journal<Type_OutputStream> : public 🍆::false_type {};

#ifndef 🍆InputStream
#define 🍆InputStream
class Type_InputStream {
public:
	🍆::WeakObject<Type_InputStream> _this;
	Type_InputStream() {
	}
	
	Type_InputStream(const Type_InputStream&) = delete;
	
	Type_InputStream(Type_InputStream&&) = delete;
	
};
#endif

template <>
struct 🍆::has_journal<Type_InputStream> : public 🍆::false_type {};

#ifndef 🍆StdOut
#define 🍆StdOut
class Type_StdOut : public Type_OutputStream {
public:
	🍆::WeakObject<Type_StdOut> _this;
	Type_StdOut() : Type_OutputStream() {
	}
	
	Type_StdOut(const Type_StdOut&) = delete;
	
	Type_StdOut(Type_StdOut&&) = delete;
	
};
#endif

template <>
struct 🍆::has_journal<Type_StdOut> : public 🍆::false_type {};

#ifndef 🍆StdErr
#define 🍆StdErr
class Type_StdErr : public Type_OutputStream {
public:
	🍆::WeakObject<Type_StdErr> _this;
	Type_StdErr() : Type_OutputStream() {
	}
	
	Type_StdErr(const Type_StdErr&) = delete;
	
	Type_StdErr(Type_StdErr&&) = delete;
	
};
#endif

template <>
struct 🍆::has_journal<Type_StdErr> : public 🍆::false_type {};

#ifndef 🍆StdIn
#define 🍆StdIn
class Type_StdIn : public Type_InputStream {
public:
	🍆::WeakObject<Type_StdIn> _this;
	Type_StdIn() : Type_InputStream() {
	}
	
	Type_StdIn(const Type_StdIn&) = delete;
	
	Type_StdIn(Type_StdIn&&) = delete;
	
};
#endif

template <>
struct 🍆::has_journal<Type_StdIn> : public 🍆::false_type {};

#ifndef 🍆FileIn
#define 🍆FileIn
class Type_FileIn : public Type_InputStream {
public:
	🍆::WeakObject<Type_FileIn> _this;
	Type_FileIn(🍆::StrongObject<Type_String> var_filename) : Type_InputStream() {
	}
	
	Type_FileIn() = delete;
	
	Type_FileIn(const Type_FileIn&) = delete;
	
	Type_FileIn(Type_FileIn&&) = delete;
	
};
#endif

template <>
struct 🍆::has_journal<Type_FileIn> : public 🍆::false_type {};

#ifndef 🍆FileOut
#define 🍆FileOut
class Type_FileOut : public Type_OutputStream {
public:
	🍆::WeakObject<Type_FileOut> _this;
	Type_FileOut(🍆::StrongObject<Type_String> var_filename) : Type_OutputStream() {
	}
	
	Type_FileOut() = delete;
	
	Type_FileOut(const Type_FileOut&) = delete;
	
	Type_FileOut(Type_FileOut&&) = delete;
	
};
#endif

template <>
struct 🍆::has_journal<Type_FileOut> : public 🍆::false_type {};

#ifndef 🍆Error
#define 🍆Error
struct Type_Error {
	🍆::Integer var_something;
	🍆::WeakObject<Type_Error> _this;
	Type_Error(🍆::StrongObject<Type_String> var_message) {
		🍆::StrongObject<Type_StdErr> var_s = (🍆::make_object<Type_StdErr>());
		((var_s)->fun_print_string((var_message)));
		((var_s)->fun_flush());
		(var_s) = (🍆::null);
		🍆::StrongObject<Type_Error> var_e;
		((var_e)->var_something) = (69LL);
	}
	
	Type_Error() = delete;
	
	Type_Error(const Type_Error&) = delete;
	
	Type_Error(Type_Error&&) = delete;
	
};
#endif

template <>
struct 🍆::has_journal<Type_Error> : public 🍆::false_type {};

