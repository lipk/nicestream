CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(nicestream)

INCLUDE_DIRECTORIES(catch/include src)
SET(CMAKE_CXX_STANDARD 11)

SET(NICE_SOURCES
    src/nfa.cpp
    src/nfa.hpp
    src/nicein.cpp
    src/nicein.hpp
    src/nicestream.hpp)

SET(TEST_SOURCES
    test_cases.cpp
    test_main.cpp)

ADD_EXECUTABLE(nice_test ${NICE_SOURCES} ${TEST_SOURCES})
