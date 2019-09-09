cmake_minimum_required (VERSION 3.13)

FILE(GLOB LIB_SOURCES *.cpp)
FILE(GLOB LIB_HEADERS *.h)
FILE(GLOB LIB_INLINE_HEADERS *.hpp)

add_library (Raven.ANPR.Recognizer OBJECT ${LIB_SOURCES} ${LIB_HEADERS} ${LIB_INLINE_HEADERS})

set_property(TARGET Raven.ANPR.Recognizer PROPERTY POSITION_INDEPENDENT_CODE 1)

# shared and static libraries built from the same object files
add_library(Raven.ANPR.Recognizer.Shared SHARED $<TARGET_OBJECTS:Raven.ANPR.Recognizer>)
add_library(Raven.ANPR.Recognizer.Static STATIC $<TARGET_OBJECTS:Raven.ANPR.Recognizer>)

set_target_properties(Raven.ANPR.Recognizer.Shared PROPERTIES OUTPUT_NAME Raven.ANPR.Recognizer) 
set_target_properties(Raven.ANPR.Recognizer.Static PROPERTIES OUTPUT_NAME Raven.ANPR.Recognizer.Static) 

find_package(Tesseract CONFIG REQUIRED)
find_package(OpenCV REQUIRED)
 
message("Client include dir (recognizer): ${CLIENT_SOURCE_DIR}/Raven.CppClient")
message("CMake Include Path(s): ${CMAKE_INCLUDE_PATH}")
target_include_directories(Raven.ANPR.Recognizer PRIVATE 
	${CMAKE_INCLUDE_PATH}
	${Tesseract_INCLUDE_DIRS} 
	${OpenCV_INCLUDE_DIRS}
	.)

target_link_libraries(Raven.ANPR.Recognizer PRIVATE libtesseract ${OpenCV_LIBS} ${CMAKE_PREFIX_PATH})