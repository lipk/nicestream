CXXFLAGS=-std=c++14 -O3 -Wall -Wextra
FILES=nicestream.cpp demo.cpp

all:
	c++ $(CXXFLAGS) $(FILES) -o nice_demo
