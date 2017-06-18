CXXFLAGS=-std=c++14 -O3 -Wall -Wextra
FILES=nicestream.cpp

test:
	c++ $(CXXFLAGS) $(FILES) test.cpp -Icatch/include -o nice_test
	exec ./nice_test

demo:
	c++ $(CXXFLAGS) $(FILES) demo.cpp -o nice_demo
