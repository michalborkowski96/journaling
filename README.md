1. Make sure you have recent clang++ (at least 8) (g++ doesn't work) and standard C++ library. In particular there might be problems with `std::variant` if you don't.
2. In the `Makefile` substitute the compiler command with the proper one.
3. `make`
4. You should see a file called `🍆`. This is the compiler.
5. `cd test`
6. `../🍆 hello_world.🍆 > main.cpp` 
7. `clang++ run.cpp -O0 -I../stdlib/cpp -std=c++17`
8. `./a.out`
