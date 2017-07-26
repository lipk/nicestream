CXXFLAGS=-std=c++11 -O3 -Wall -Wextra -Werror -Isrc
FILES=src/nicestream.cpp src/nfa.cpp
CXX ?= c++

test:
	$(CXX) $(CXXFLAGS) $(FILES) test.cpp -Icatch/include -g -o nice_test 
	exec ./nice_test

test-build-only:
	$(CXX) $(CXXFLAGS) $(FILES) test.cpp -Icatch/include -g -o nice_test
