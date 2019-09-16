cmake_minimum_required (VERSION 3.13)

cmake_policy(SET CMP0079 NEW) # allow linking from ANOTHER folder
FILE(GLOB TEST_FIXTURES *.cpp)

include(BoostTestHelper.cmake)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)
find_package(OpenCV REQUIRED CONFIG)

foreach(_file ${TEST_FIXTURES})
	add_boost_test(${_file} Raven.ANPR.Recognizer Raven.CppClient)
endforeach()
 
add_custom_command(
			TARGET ${TEST_EXECUTABLE_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Raven.ANPR.Tests/test_license_plate.jpg ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME})