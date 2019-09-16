cmake_minimum_required (VERSION 3.13)
project (Raven.ANPR VERSION 0.0.1 LANGUAGES C CXX)
include (CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
	set(CMAKE_CXX_FLAGS_DEBUG "/MTd /Zi /Ob0 /Od /RTC1")
	set(CMAKE_CXX_FLAGS_RELEASE "/MT /O2 /Ob2 /DNDEBUG")	
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)

list(APPEND CMAKE_CXX_SOURCE_FILE_EXTENSIONS c)
 
if(NOT DEFINED ARCH)
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	  MESSAGE("+ 64 bit architecture") 
	  set(ARCH "x64") 
	 else()
	  MESSAGE("+ 32 bit architecture")  
	  set(ARCH "x86") 
	endif()
endif() 
  
if(WIN32)
#this is most likely due to a bug in vcpkg configuration, we need to set these manually...
	set(VCPKG_TARGET_TRIPLET "${ARCH}-windows-static")	
elseif(UNIX)
	set(VCPKG_TARGET_TRIPLET "${ARCH}-linux")
elseif(APPLE)
	set(VCPKG_TARGET_TRIPLET "${ARCH}-osx")
else()
#TODO: add arm triplets too, test the builds on arm dockers
	message(FATAL_ERROR "Can compile only on Unix, Windows or MacOS operating systems.")
endif()

#library names for vcpkg to install
set(PROJECT_DEPENDENCIES boost-core boost-test tesseract leptonica opencv[jpeg,png] nlohmann-json curl)

#make sure to install and build vcpkg to enable them for the solution
if(NOT DEFINED ${CMAKE_TOOLCHAIN_FILE})

#first find proper path for vcpkg
  if(NOT DEFINED ENV{VCPKG_ROOT})
	if(WIN32)	
	  set(VCPKG_ROOT $ENV{HOMEDRIVE}$ENV{HOMEPATH}vcpkg)
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

 

IF(CMAKE_BUILD_TYPE MATCHES Debug)
	message("++ This is a Debug build")
	set (CMAKE_CONFIGURATION_TYPES, "Debug")
	list (APPEND CMAKE_LIBRARY_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/lib")
	set(LIBRARY_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/lib")
	list (APPEND CMAKE_LIBRARY_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/lib/manual-link")
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
	message("++ This is a Release build")
	set (CMAKE_CONFIGURATION_TYPES, "Release")
	list (APPEND CMAKE_LIBRARY_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib")
	set(LIBRARY_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib")
	list (APPEND CMAKE_LIBRARY_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib/manual-link")
ELSE()
	message("++ No build type specified, defaulting to release build")
	list (APPEND CMAKE_LIBRARY_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib")
	set(LIBRARY_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib")
	list (APPEND CMAKE_LIBRARY_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/lib/manual-link")
	set (CMAKE_BUILD_TYPE "Release")
ENDIF()

include(ExternalProject)
ExternalProject_Add(RavenDBCppClient
  PREFIX "${PROJECT_SOURCE_DIR}/libs"
  GIT_REPOSITORY https://github.com/myarichuk/ravendb-cpp-client.git
  GIT_TAG        master
  STEP_TARGETS   build
  UPDATE_COMMAND "" 
  INSTALL_COMMAND ""  
)
 
ExternalProject_Get_property(RavenDBCppClient BINARY_DIR)
ExternalProject_Get_property(RavenDBCppClient SOURCE_DIR)
 
message("RavenDB.CppClient sources dir: ${SOURCE_DIR}")
message("RavenDB.CppClient binaries dir: ${BINARY_DIR}")

add_library(Raven_CppClient_static STATIC IMPORTED GLOBAL)
set(CLIENT_SOURCE_DIR ${SOURCE_DIR} CACHE STRING "")

list (APPEND CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/share")

list (APPEND CMAKE_INCLUDE_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include")
list (APPEND CMAKE_INCLUDE_PATH "${VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/include/nlohmann")
list (APPEND CMAKE_INCLUDE_PATH "${SOURCE_DIR}/Raven.CppClient")
list (APPEND CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/Raven.ANPR.Recognizer")
list (APPEND CMAKE_INCLUDE_PATH "${Boost_INCLUDE_DIR}")
list (APPEND CMAKE_LIBRARY_PATH "${BINARY_DIR}/Raven.CppClient")

link_directories(${CMAKE_LIBRARY_PATH})
include_directories(${CMAKE_INCLUDE_PATH})

# Include sub-projects
add_subdirectory ("Raven.ANPR.Recognizer")
add_subdirectory ("Raven.ANPR.Tests") 

add_dependencies(Raven.ANPR.Recognizer RavenDBCppClient)
