CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(nicestream)

INCLUDE_DIRECTORIES(catch/single_include src)
SET(CMAKE_CXX_STANDARD 11)

SET(NICE_SOURCES
    src/nfa.cpp
    src/nfa.hpp
    src/nicein.cpp
    src/nicein.hpp
    src/nicestream.hpp)

SET(TEST_SOURCES
    test/input_tests.cpp
    test/output_tests.cpp
    test/test_main.cpp)

ADD_EXECUTABLE(nice_test ${NICE_SOURCES} ${TEST_SOURCES})

ADD_CUSTOM_TARGET(format COMMAND
    clang-format -style=file -i ${NICE_SOURCES} ${TEST_SOURCES})
