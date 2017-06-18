CXXFLAGS=-std=c++11 -O3 -Wall -Wextra -Werror
FILES=nicestream.cpp

test:
	c++ $(CXXFLAGS) $(FILES) test.cpp -Icatch/include -o nice_test
	exec ./nice_test

test-build-only:
	c++ $(CXXFLAGS) $(FILES) test.cpp -Icatch/include -o nice_test

demo:
	c++ $(CXXFLAGS) $(FILES) demo.cpp -o nice_demo
