# CMakeList.txt : Top-level CMake project file, do global configuration
# since we use vcpkg, make sure to include -DCMAKE_TOOLCHAIN_FILE=C:/Work/Raven.ANPR/Libs/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake_minimum_required (VERSION 3.10)

project (Raven.ANPR VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

#library names for vcpkg to install
set(PROJECT_DEPENDENCIES boost-core boost-test)

#make sure to install and build vcpkg to enable them for the solution
if(NOT DEFINED ${CMAKE_TOOLCHAIN_FILE})

#first find proper path for vcpkg
  if(NOT DEFINED ENV{VCPKG_ROOT})
	if(WIN32)
	  set(VCPKG_ROOT $ENV{HOMEDRIVE}$ENV{HOMEPATH}/vcpkg)
	else()
	  set(VCPKG_ROOT $ENV{HOME}/.vcpkg)
	endif()
  else()
	set(VCPKG_ROOT $ENV{VCPKG_ROOT})
  endif()

#then make sure vcpkg is up to date
  if(NOT EXISTS ${VCPKG_ROOT})
	message("Cloning vcpkg in ${VCPKG_ROOT}")
	execute_process(COMMAND git clone https://github.com/Microsoft/vcpkg.git ${VCPKG_ROOT})
	# If a reproducible build is desired (and potentially old libraries are # ok), uncomment the
	# following line and pin the vcpkg repository to a specific githash.
	# execute_process(COMMAND git checkout 745a0aea597771a580d0b0f4886ea1e3a94dbca6 WORKING_DIRECTORY ${VCPKG_ROOT})
  else()
	# The following command has no effect if the vcpkg repository is in a detached head state.
	message("Auto-updating vcpkg in ${VCPKG_ROOT}")
	execute_process(COMMAND git pull WORKING_DIRECTORY ${VCPKG_ROOT})
	execute_process(COMMAND ${VCPKG_EXEC} update)
  endif()

  if(NOT EXISTS ${VCPKG_ROOT}/README.md)
	message(FATAL_ERROR "***** FATAL ERROR: Could not clone vcpkg, make sure git is installed and configured properly *****")
  endif()

  #compile vcpkg if this is a fresh installation
  #GTEST_ROOT
  if(WIN32)
	set(BOOST_INCLUDEDIR ${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include)
	set(VCPKG_EXEC ${VCPKG_ROOT}/vcpkg.exe)
	set(VCPKG_BOOTSTRAP ${VCPKG_ROOT}/bootstrap-vcpkg.bat)
  else()
	set(VCPKG_EXEC ${VCPKG_ROOT}/vcpkg)
	set(VCPKG_BOOTSTRAP ${VCPKG_ROOT}/bootstrap-vcpkg.sh)
  endif()

  if(NOT EXISTS ${VCPKG_EXEC})
	message("Bootstrapping vcpkg in ${VCPKG_ROOT}")
	execute_process(COMMAND ${VCPKG_BOOTSTRAP} WORKING_DIRECTORY ${VCPKG_ROOT})
  endif()

  if(NOT EXISTS ${VCPKG_EXEC})
	message(FATAL_ERROR "***** FATAL ERROR: Could not bootstrap vcpkg *****")
  endif()

  #make sure to include third party stuff with CMake compilation
  set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "")

  message(STATUS "***** Checking project third party dependencies in ${VCPKG_ROOT} *****")

  #actually install vcpkg libraries
  execute_process(COMMAND ${VCPKG_EXEC} install --triplet ${VCPKG_TARGET_TRIPLET} ${PROJECT_DEPENDENCIES} WORKING_DIRECTORY ${VCPKG_ROOT})
endif()

enable_testing()

# Include sub-projects.
add_subdirectory ("Raven.ANPR.Recognizer")
add_subdirectory ("Raven.ANPR.Tests")

#message("Setting Boost include directories to '${Boost_INCLUDE_DIR}'")
#include_directories(${Boost_INCLUDE_DIR})

