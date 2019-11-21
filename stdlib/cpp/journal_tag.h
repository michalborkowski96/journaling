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
	void basefun_t_(🍆::StrongObject<Type_JournalTag_<T>> var_ttt, 🍆::StrongObject<T> var_tt) {
		((var_ttt)->fun_t((var_tt)));
	}

	virtual void privfun_t_(🍆::StrongObject<Type_JournalTag_<T>> var_ttt, 🍆::StrongObject<T> var_tt) {
		(basefun_t_(var_ttt, var_tt));
	}
	virtual void fun_t_(🍆::StrongObject<Type_JournalTag_<T>> var_ttt, 🍆::StrongObject<T> var_tt) {
		(basefun_t_(var_ttt, var_tt));
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
	void basefun_t_(🍆::StrongObject<Type_JournalTag_<🍆::Integer>>, 🍆::StrongObject<🍆::Integer>) {}

	virtual void privfun_t_(🍆::StrongObject<Type_JournalTag_<🍆::Integer>> var_ttt, 🍆::StrongObject<🍆::Integer> var_tt) {
		(basefun_t_(var_ttt, var_tt));
	}
	virtual void fun_t_(🍆::StrongObject<Type_JournalTag_<🍆::Integer>> var_ttt, 🍆::StrongObject<🍆::Integer> var_tt) {
		(basefun_t_(var_ttt, var_tt));
	}
};

#define 🍆_Default_JournalTag
