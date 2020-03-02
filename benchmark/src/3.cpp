#include <iostream>
#include <vector>

[[clang::optnone]]
int main(){
	long long total = 0;
	std::cin >> total;
	std::vector<long long> nums;
	for(long long i = 0; i < total; ++i) {
		nums.push_back(i);
	}
	auto sum = [nums](){
		long long s = 0;
		for(long long i : nums) {
			s += i;
		}
		return s;
	};
	for(long long& i : nums) {
		i += total;
	}
	long long s = 0;
	for(long long i : nums) {
		s += i;
	}
	std::cout << s << '\n';
	std::cout << sum() << '\n';
}