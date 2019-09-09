cmake_minimum_required (VERSION 3.13)

cmake_policy(SET CMP0079 NEW) # allow linking from ANOTHER folder
FILE(GLOB TEST_FIXTURES *.cpp)

include(BoostTestHelper.cmake)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)
include_directories(${CMAKE_INCLUDE_PATH} ${Tesseract_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

foreach(_file ${TEST_FIXTURES})
	add_boost_test(${_file} Raven.ANPR.Recognizer ${CMAKE_PREFIX_PATH})
endforeach()