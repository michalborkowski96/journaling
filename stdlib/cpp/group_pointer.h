#ifndef GROUP_POINTER_H
#define GROUP_POINTER_H

#include <algorithm>
#include <cstddef>

namespace {
	template<typename T>
	class ControlBlock {
		static_assert(sizeof(T*) == 8, "Due to magic, this works only on 64-bit architectures.");
		union {
			struct {
				char garbage[sizeof(void*) - 1];
				char points_to_a_control_block;
			} raw;
			ControlBlock* next_control_block_pointer;
			T* object_pointer;
		} pointer;
		static_assert(sizeof(pointer) == 8, "Woah padding!");
		ControlBlock* previous_control_block_pointer;
		size_t shared_count;
		size_t weak_count;
		~ControlBlock() = default;
		ControlBlock* get_next_control_block() const {
			static const size_t mask = [](){size_t i = 0; --i; char* ic = (char*) &i; ic[7] = 0; return i;}();
			ControlBlock* p = pointer.next_control_block_pointer;
			return reinterpret_cast<ControlBlock*>(reinterpret_cast<size_t>(p) & mask);
		}
		ControlBlock* get_last_block() {
			if(pointer.raw.points_to_a_control_block) {
				return get_next_control_block()->get_last_block();
			} else {
				return this;
			}
		}
	public:
		ControlBlock() = delete;
		ControlBlock(const ControlBlock&) = delete;
		ControlBlock(ControlBlock&&) = delete;
		ControlBlock(T* pointer) : pointer({.object_pointer = pointer}), previous_control_block_pointer(nullptr), shared_count(1), weak_count(0) {}
		void get(ControlBlock*& last_block, T*& object) {
			last_block = get_last_block();
			object = last_block->pointer.object_pointer;
		}
		bool valid() const {
			const ControlBlock* cb = this;
			while(cb) {
				if(cb->shared_count != 0) {
					return true;
				}
				cb = cb->previous_control_block_pointer;
			}
			cb = this;
			while(true) {
				if(cb->shared_count != 0) {
					return true;
				}
				if(cb->pointer.raw.points_to_a_control_block) {
					cb = cb->get_next_control_block();
				} else {
					return false;
				}
			}
		}
		void increase_shared_count(){
			++shared_count;
		}
		void increase_weak_count(){
			++weak_count;
		}
		void decrease_shared_count(){
			if((--shared_count) == 0) {
				try_free();
				if(weak_count == 0) {
					remove();
				}
			}
		}
		void decrease_weak_count(){
			if((--weak_count) == 0 && shared_count == 0) {
				remove();
			}
		}

		void try_free() {
			if(valid()) {
				return;
			}
			ControlBlock* cb = get_last_block();
			cb->shared_count = 1;
			delete cb->pointer.object_pointer;
			cb->shared_count = 0;
		}

		void remove() {
			if(previous_control_block_pointer) {
				previous_control_block_pointer->pointer = pointer;
			}
			if(pointer.raw.points_to_a_control_block) {
				get_next_control_block()->previous_control_block_pointer = previous_control_block_pointer;
			}
			delete this;
		}
		void group_assume(ControlBlock* o) {
			while(o->previous_control_block_pointer) {
				o = o->previous_control_block_pointer;
			}
			ControlBlock* last = get_last_block();
			delete last->pointer.object_pointer;
			last->pointer.next_control_block_pointer = o;
			last->pointer.raw.points_to_a_control_block = true;
		}
	};
}

template<typename T>
class GroupWeakPointer;

template<typename T>
class GroupSharedPointer {
	ControlBlock<T>* control_block;
	friend class GroupWeakPointer<T>;
	GroupSharedPointer(ControlBlock<T>* control_block) : control_block(control_block) {
		if(control_block) {
			if(control_block->valid()) {
				control_block->increase_shared_count();
			} else {
				this->control_block = nullptr;
			}
		}
	}
public:
	using element_type = T;
	GroupSharedPointer() = delete;
	~GroupSharedPointer() {
		if(control_block) {
			control_block->decrease_shared_count();
		}
	}
	GroupSharedPointer(std::nullptr_t) : control_block(nullptr) {}
	explicit GroupSharedPointer(T* ptr) try : control_block(new ControlBlock<T>(ptr)) {} catch(...) {delete ptr; throw;}
	GroupSharedPointer(const GroupSharedPointer& o) : control_block(o.control_block) {
		if(control_block) {
			control_block->increase_shared_count();
		}
	}
	GroupSharedPointer(GroupSharedPointer&& o) : control_block(o.control_block) {
		if(o.control_block) {
			o.control_block = nullptr;
		}
	}
	GroupSharedPointer& operator=(std::nullptr_t) {
		if(control_block) {
			control_block->decrease_shared_count();
			control_block = nullptr;
		}
		return *this;
	}
	GroupSharedPointer& operator=(const GroupSharedPointer& o) {
		GroupSharedPointer p(o);
		swap(*this, p);
		return *this;
	}
	GroupSharedPointer& operator=(GroupSharedPointer&& o) {
		*this = nullptr;
		swap(*this, o);
		return *this;
	}
	static void swap(GroupSharedPointer& a, GroupSharedPointer& b) {
		std::swap(a.control_block, b.control_block);
	}
	T* get() {
		T* ptr;
		ControlBlock<T>* new_control_block;
		control_block->get(new_control_block, ptr);
		if(new_control_block != control_block) {
			new_control_block->increase_shared_count();
			control_block->decrease_shared_count();
			control_block = new_control_block;
		}
		return ptr;
	}
	T* operator->() {
		return get();
	}
	T& operator*() {
		return *get();
	}
	void group_assume(const GroupSharedPointer<T>& o) {
		control_block->group_assume(o.control_block);
	}
	explicit operator bool() const {
		return control_block;
	}
};

template<typename T>
class GroupWeakPointer {
	ControlBlock<T>* control_block;
public:
	using element_type = T;
	GroupWeakPointer() = delete;
	~GroupWeakPointer() {
		if(control_block) {
			control_block->decrease_weak_count();
		}
	}
	GroupWeakPointer(std::nullptr_t) : control_block(nullptr) {}
	GroupWeakPointer(const GroupSharedPointer<T>& ptr) : control_block(ptr.control_block) {
		if(control_block) {
			control_block->increase_weak_count();
		}
	}
	GroupWeakPointer(const GroupWeakPointer& o) : control_block(o.control_block) {
		if(control_block) {
			control_block->increase_weak_count();
		}
	}
	GroupWeakPointer(GroupWeakPointer&& o) : control_block(o.control_block) {
		if(o.control_block) {
			o.control_block = nullptr;
		}
	}
	GroupWeakPointer& operator=(const GroupWeakPointer& o) {
		GroupWeakPointer p(o);
		swap(*this, p);
		return *this;
	}
	GroupWeakPointer& operator=(GroupWeakPointer&& o) {
		*this = nullptr;
		swap(*this, o);
		return *this;
	}
	GroupWeakPointer& operator=(std::nullptr_t) {
		if(control_block) {
			control_block->decrease_weak_count();
			control_block = nullptr;
		}
		return *this;
	}
	static void swap(GroupWeakPointer& a, GroupWeakPointer& b) {
		std::swap(a.control_block, b.control_block);
	}
	GroupSharedPointer<T> lock() const {
		return GroupSharedPointer<T>(control_block);
	}
	explicit operator bool() const {
		return control_block;
	}
};

#endif
