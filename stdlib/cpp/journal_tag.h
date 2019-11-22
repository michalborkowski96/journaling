template<typename T>
class Type_JournalTag_;

template<typename T>
class Type_JournalTag {
public:
	🍆::WeakObject<Type_JournalTag> this_ptr;
	🍆::StrongObject<Type_JournalTag> get_this() {
		return 🍆::scast<Type_JournalTag>(this_ptr);
	}
	🍆::StrongObject<T> object;
	🍆::Journal::Tag tag;
	virtual ~Type_JournalTag() = default;

	Type_JournalTag(WeakObject<Type_JournalTag> this_ptr, 🍆::StrongObject<T> var_t) : this_ptr(this_ptr), object(var_t), tag(var_t->journal.get_tag()){
	}

	Type_JournalTag() = delete;

	Type_JournalTag(const Type_JournalTag&) = delete;

	Type_JournalTag(Type_JournalTag&&) = delete;

	void basefun_bring_back() {
		object->journal.goto_tag(tag);
	}

	virtual void privfun_bring_back() {
		(basefun_bring_back());
	}
	virtual void fun_bring_back() {
		(basefun_bring_back());
	}
};

template<>
class Type_JournalTag<🍆::Integer> {
public:
	🍆::WeakObject<Type_JournalTag> this_ptr;
	🍆::StrongObject<Type_JournalTag> get_this() {
		return 🍆::scast<Type_JournalTag>(this_ptr);
	}
	virtual ~Type_JournalTag() = default;

	Type_JournalTag(WeakObject<Type_JournalTag> this_ptr, 🍆::StrongObject<🍆::Integer>) : this_ptr(this_ptr) {
	}

	Type_JournalTag() = delete;

	Type_JournalTag(const Type_JournalTag&) = delete;

	Type_JournalTag(Type_JournalTag&&) = delete;

	void basefun_bring_back() {}

	virtual void privfun_bring_back() {
		(basefun_bring_back());
	}
	virtual void fun_bring_back() {
		(basefun_bring_back());
	}
};

#define 🍆_Default_JournalTag
