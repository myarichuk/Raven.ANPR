cmake_minimum_required (VERSION 3.13)

cmake_policy(SET CMP0079 NEW) # allow linking from ANOTHER folder
FILE(GLOB TEST_FIXTURES *.cpp)

include(BoostTestHelper.cmake)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

foreach(_file ${TEST_FIXTURES})
	add_boost_test(${_file} Raven.ANPR.Recognizer)
endforeach()