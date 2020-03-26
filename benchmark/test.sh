perform_test_🍆 () {
	mkdir bin
	cp src/run.cpp bin/run.cpp
	../🍆 src/"$1".🍆 2>/dev/null >bin/test.cpp
	sed -i '/void basefun_test() {/c\[[clang::optnone]] void basefun_test() {' bin/test.cpp
	clang++ -std=c++17 -O3 bin/run.cpp -o bin/test.out -I../stdlib/cpp
	echo "$2" | /usr/bin/time -v bin/test.out 2>&1 | cat --
	rm -rf bin
}

perform_test_hs () {
        mkdir bin
	cp src/"$1".hs bin/test.hs
        ghc -O0 bin/test.hs -o bin/test.out 2>&1 >/dev/null
        echo "$2" | /usr/bin/time -v bin/test.out 2>&1 | cat --
        rm -rf bin
}

perform_test_c++ () {
        mkdir bin
        clang++ -std=c++17 -O3 src/"$1".cpp -o bin/test.out
        echo "$2" | /usr/bin/time -v bin/test.out 2>&1 | cat --
        rm -rf bin
}

get_user_time () {
	e=$(echo "$1" | grep terminated\ by\ signal)
	if [ -z "$e" ]
	then
		echo "$1" | grep User\ time\ \(seconds\):\ .* | cut -c 23- --
	else
		echo "$e"
	fi
}

get_used_memory () {
	expr $(echo "$1" | grep Maximum\ resident\ set\ size\ \(kbytes\):\ .* | cut -c 38- --) / 1024
}
