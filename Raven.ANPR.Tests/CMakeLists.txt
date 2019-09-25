cmake_minimum_required (VERSION 3.13)

cmake_policy(SET CMP0079 NEW) # allow linking from ANOTHER folder
FILE(GLOB TEST_FIXTURES *.cpp)

include(BoostTestHelper.cmake)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)
find_package(OpenCV REQUIRED CONFIG)
find_package(Tesseract CONFIG REQUIRED)

link_libraries(libtesseract)

foreach(_file ${TEST_FIXTURES})
	add_boost_test(${_file} Raven.ANPR.Recognizer Raven.CppClient)
endforeach()
 
add_custom_command(
			TARGET ${TEST_EXECUTABLE_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Raven.ANPR.Tests/test_license_plate.jpg ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME})

add_custom_command(
			TARGET ${TEST_EXECUTABLE_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Raven.ANPR.Tests/test_license_plate2.jpg ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME})

add_custom_command(
			TARGET ${TEST_EXECUTABLE_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Raven.ANPR.Tests/test_license_plate3.jpg ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME})

add_custom_command(
			TARGET ${TEST_EXECUTABLE_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Raven.ANPR.Tests/test_license_plate4.jpg ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME})

add_custom_command(
			TARGET ${TEST_EXECUTABLE_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Raven.ANPR.Tests/test_license_plate5.jpg ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME})

add_custom_command(
			TARGET ${TEST_EXECUTABLE_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Raven.ANPR.Tests/test_license_plate_invalid.jpg ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME})

			
add_custom_command(
			TARGET ${TEST_EXECUTABLE_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Raven.ANPR.Tests/test_license_plate_missing.jpg ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME})

add_custom_command(
			TARGET ${TEST_EXECUTABLE_TARGET} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME}/tessdata
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Raven.ANPR.Recognizer/eng.traineddata ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME}/tessdata)
