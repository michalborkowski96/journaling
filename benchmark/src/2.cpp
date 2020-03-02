#include <iostream>
#include <list>

struct BoxedInt {
	long long* value;
	BoxedInt(long long value) : value(new long long(value)) {}
	~BoxedInt() {
		delete value;
	}
	BoxedInt(const BoxedInt& o) : value(new long long(*o.value)) {}
};

[[clang::optnone]]
int main(){
	long long total = 0;
	std::cin >> total;
	std::list<BoxedInt> nums;
	for(long long i = 0; i < total; ++i) {
		nums.push_back(i);
	}
	auto sum = [nums](){
		long long s = 0;
		for(const BoxedInt& i : nums) {
			s += *i.value;
		}
		return s;
	};
	for(BoxedInt& i : nums) {
		*i.value += total;
	}
	long long s = 0;
	for(const BoxedInt& i : nums) {
		s += *i.value;
	}
	std::cout << s << '\n';
	std::cout << sum() << '\n';
}
