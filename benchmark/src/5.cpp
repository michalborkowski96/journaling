#include <iostream>
#include <list>
#include <functional>

[[clang::optnone]]
int main(){
	long long total1 = 0, total2 = 0;
	std::cin >> total1 >> total2;
	std::list<long long> nums;
	std::list<std::function<long long()>> sums;
	for(long long i = 0; i < total1; ++i) {
		nums.push_back(i);
	}
	for(long long i = 0; i < total2; ++i) {
		nums.push_back(i);
		sums.push_back([nums](){
			long long s = 0;
			for(long long i : nums) {
				s += i;
			}
			return s;
		});
	}
	for(long long& i : nums) {
		i += total2;
	}
	long long s = 0;
	for(long long i : nums) {
		s += i;
	}
	long long ss = 0;
	long long i = 0;
	for(auto it = sums.begin(); it != sums.end(); ++it) {
		if(i % 2) {
			ss -= (*it)();
		} else {
			ss += (*it)();
		}
		++i;
	}
	std::cout << s << '\n';
	std::cout << ss << '\n';
}
