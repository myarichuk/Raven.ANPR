cmake_minimum_required (VERSION 3.10)

add_library (Raven.ANPR.Recognizer "Raven.ANPR.Recognizer.cpp" "Raven.ANPR.Recognizer.h")

find_package(Tesseract CONFIG REQUIRED)
target_include_directories(Raven.ANPR.Recognizer PRIVATE {Tesseract_INCLUDE_DIRS})
target_link_libraries(Raven.ANPR.Recognizer PRIVATE libtesseract)

find_package(OpenCV REQUIRED)

target_include_directories(Raven.ANPR.Recognizer PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(Raven.ANPR.Recognizer PRIVATE ${OpenCV_LIBS})

