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
	🍆::WeakObject<Type_String> _this;
public:
};
#endif

template <>
struct Type_ListElement_<🍆::Integer, 🍆::Integer> {
	🍆::WeakObject<Type_ListElement_> _this;
	🍆::StrongObject<🍆::Integer> var_element_;
	🍆::StrongObject<Type_ListElement_> var_next_;
	🍆::WeakObject<Type_ListElement_> var_prev_;
};

template <>
class Type_SimpleList_<🍆::Integer, 🍆::Integer> {
	🍆::WeakObject<Type_SimpleList_> _this;
	🍆::Integer var_size_;
	🍆::StrongObject<Type_ListElement_<🍆::Integer, 🍆::Integer>> var_begin_;
	🍆::StrongObject<Type_ListElement_<🍆::Integer, 🍆::Integer>> var_end_;
public:
};

#ifndef 🍆OutputStream
#define 🍆OutputStream
class Type_OutputStream {
	🍆::WeakObject<Type_OutputStream> _this;
public:
};
#endif

#ifndef 🍆InputStream
#define 🍆InputStream
class Type_InputStream {
	🍆::WeakObject<Type_InputStream> _this;
public:
};
#endif

#ifndef 🍆StdOut
#define 🍆StdOut
class Type_StdOut : public Type_OutputStream {
	🍆::WeakObject<Type_StdOut> _this;
public:
};
#endif

#ifndef 🍆StdErr
#define 🍆StdErr
class Type_StdErr : public Type_OutputStream {
	🍆::WeakObject<Type_StdErr> _this;
public:
};
#endif

#ifndef 🍆StdIn
#define 🍆StdIn
class Type_StdIn : public Type_InputStream {
	🍆::WeakObject<Type_StdIn> _this;
public:
};
#endif

#ifndef 🍆FileIn
#define 🍆FileIn
class Type_FileIn : public Type_InputStream {
	🍆::WeakObject<Type_FileIn> _this;
public:
};
#endif

#ifndef 🍆FileOut
#define 🍆FileOut
class Type_FileOut : public Type_OutputStream {
	🍆::WeakObject<Type_FileOut> _this;
public:
};
#endif

#ifndef 🍆Error
#define 🍆Error
struct Type_Error {
	🍆::WeakObject<Type_Error> _this;
	🍆::Integer var_something;
};
#endif

