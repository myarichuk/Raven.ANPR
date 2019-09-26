cmake_minimum_required (VERSION 3.13)

FILE(GLOB_RECURSE LIB_SOURCES *.cpp)
FILE(GLOB_RECURSE LIB_HEADERS *.h)
FILE(GLOB_RECURSE LIB_INLINE_HEADERS *.hpp)

add_library(Raven.ANPR.Recognizer STATIC ${LIB_SOURCES} ${LIB_HEADERS} ${LIB_INLINE_HEADERS})

find_package(OpenCV REQUIRED CONFIG)
find_package(Leptonica CONFIG REQUIRED)
find_package(Tesseract CONFIG REQUIRED)

foreach(header in ${LIB_HEADERS})
	get_filename_component(HEADER_FOLDER ${header} DIRECTORY)
	list(APPEND CMAKE_INCLUDE_PATH ${HEADER_FOLDER})
endforeach()

foreach(header in ${LIB_INLINE_HEADERS})
	get_filename_component(HEADER_FOLDER ${header} DIRECTORY)
	list(APPEND CMAKE_INCLUDE_PATH ${HEADER_FOLDER})
endforeach()

target_include_directories(Raven.ANPR.Recognizer PRIVATE ${OpenCV_INCLUDE_DIRS} ${CMAKE_INCLUDE_PATH})
target_link_libraries(Raven.ANPR.Recognizer Raven.CppClient ${OpenCV_LIBS} leptonica libtesseract)

add_custom_command(
			TARGET Raven.ANPR.Recognizer POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME}/tessdata
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Raven.ANPR.Recognizer/eng.traineddata ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME}/tessdata
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Raven.ANPR.Recognizer/eng.traineddata.best ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME}/tessdata
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/Raven.ANPR.Recognizer/osd.traineddata ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_DIR_NAME}/tessdata)

