# CMakeList.txt : Top-level CMake project file, do global configuration
# since we use vcpkg, make sure to include -DCMAKE_TOOLCHAIN_FILE=C:/Work/Raven.ANPR/Libs/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake_minimum_required (VERSION 3.8)

project ("Raven.ANPR" VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

# Include sub-projects.
add_subdirectory ("Raven.ANPR.Recognizer")
add_subdirectory ("Raven.ANPR.Tests")

