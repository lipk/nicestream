CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(nicestream)

INCLUDE_DIRECTORIES(catch/include src)
SET(CMAKE_CXX_STANDARD 11)
SET(SOURCES test_cases.cpp test_main.cpp src/nicestream.cpp src/nfa.cpp src/nicestream.hpp src/nfa.hpp)

ADD_EXECUTABLE(nice_test ${SOURCES})
