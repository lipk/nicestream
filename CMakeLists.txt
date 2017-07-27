CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(nicestream)

INCLUDE_DIRECTORIES(catch/include src)
SET(CMAKE_CXX_STANDARD 11)
SET(SOURCES test_cases.cpp test_main.cpp src/nicestream.cpp src/nfa.cpp src/nicestream.hpp src/nfa.hpp)

SET(NICE_SOURCES
    src/nfa.cpp
    src/nfa.hpp
    src/nicestream.cpp
    src/nicestream.hpp)

SET(TEST_SOURCES
    test_cases.cpp
    test_main.cpp)

ADD_EXECUTABLE(nice_test ${NICE_SOURCES} ${TEST_SOURCES})
