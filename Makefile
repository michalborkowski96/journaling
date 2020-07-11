.DEFAULT_GOAL := 🍆
CXX_FLAGS := -std=c++17 -O0 -g -Wall -Wextra -march=native
CXX := clang++

CPP := $(wildcard *.cpp)
OBJ := $(CPP:%.cpp=%.o)
DEP := $(OBJ:%.o=%.d)

-include $(DEP)

%.o: %.cpp
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

🍆: $(OBJ)
	$(CXX) -o $@ $(OBJ) $(CXX_FLAGS)

.PHONY: clean
clean:
	-rm -f $(OBJ) $(DEP)
	-rm -f 🍆
