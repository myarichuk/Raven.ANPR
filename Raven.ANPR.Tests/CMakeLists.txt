cmake_minimum_required (VERSION 3.8)

cmake_policy(SET CMP0079 NEW) # allow linking from ANOTHER folder

include(BoostTestHelper.cmake)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

add_boost_test(BasicTests.cpp ${CMAKE_CURRENT_BINARY_DIR}/../Raven.ANPR.Recognizer/Raven.ANPR.Recognizer.lib)
add_boost_test(AdvancedTests.cpp ${CMAKE_CURRENT_BINARY_DIR}/../Raven.ANPR.Recognizer/Raven.ANPR.Recognizer.lib)