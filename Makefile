CXXFLAGS=-std=c++11 -O3 -Wall -Wextra -Werror
FILES=nicestream.cpp

test:
	clang++ $(CXXFLAGS) $(FILES) test.cpp -Icatch/include -o nice_test
	exec ./nice_test

test-build-only:
	clang++ $(CXXFLAGS) $(FILES) test.cpp -Icatch/include -o nice_test

demo:
	clang++ $(CXXFLAGS) $(FILES) demo.cpp -o nice_demo
