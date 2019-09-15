cmake_minimum_required (VERSION 3.13)

FILE(GLOB LIB_SOURCES *.cpp)
FILE(GLOB LIB_HEADERS *.h)
FILE(GLOB LIB_INLINE_HEADERS *.hpp)

add_library(Raven.ANPR.Recognizer STATIC ${LIB_SOURCES} ${LIB_HEADERS} ${LIB_INLINE_HEADERS})

find_package(OpenCV REQUIRED CONFIG)

target_include_directories(Raven.ANPR.Recognizer PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(Raven.ANPR.Recognizer Raven.CppClient ${OpenCV_LIBS})
