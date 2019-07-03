cmake_minimum_required (VERSION 3.8)

ENABLE_TESTING()

ADD_EXECUTABLE(unit_tests "tests.cpp" "tests.h")
ADD_TEST(BasicTest unit_tests)