This code is a bit of a mess, mostly because it was supposed to be for research. For details see http://students.mimuw.edu.pl/~mb370727/JournalingEN.pdf

1. Make sure you have recent clang++ (at least 8) (g++ doesn't work) and standard C++ library. In particular there might be problems with `std::variant` if you don't.
2. `make`
3. You should see a file called `🍆`. This is the compiler.
4. `cd test`
5. `../🍆 hello_world.🍆 > main.cpp` 
6. `clang++ run.cpp -O0 -I../stdlib/cpp -std=c++17`
7. `./a.out`
