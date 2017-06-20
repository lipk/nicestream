CXXFLAGS=-std=c++11 -O3 -Wall -Wextra -Werror -Isrc
FILES=src/nicestream.cpp src/nfa.cpp

test:
	clang++ $(CXXFLAGS) $(FILES) test.cpp -Icatch/include -g -o nice_test 
	exec ./nice_test

test-build-only:
	clang++ $(CXXFLAGS) $(FILES) test.cpp -Icatch/include -g -o nice_test

demo:
	clang++ $(CXXFLAGS) $(FILES) demo.cpp -o nice_demo
