set(FETCHCONTENT_QUIET false)
include(FetchContent)

set (CPP_CLIENT_SRC "${PROJECT_SOURCE_DIR}/libs/ravendb-cpp-client")
 
FetchContent_Declare(
  ravendb_cpp_client
  GIT_REPOSITORY https://github.com/ravendb/ravendb-cpp-client.git
  GIT_TAG        master
  SOURCE_DIR ${CPP_CLIENT_SRC}/repository SUBBUILD_DIR ${CPP_CLIENT_SRC}/subbuild  BINARY_DIR ${CPP_CLIENT_SRC}/binary
  )

FetchContent_GetProperties(ravendb_cpp_client)
if(NOT ravendb_cpp_client_POPULATED)  
  FetchContent_Populate(ravendb_cpp_client)
endif()

if(WIN32)
	FIND_PROGRAM(VSWHERE_EXE vswhere PATHS ${CMAKE_BINARY_DIR}/tools)
	IF(VSWHERE_EXE)
		MESSAGE("-- Found vswhere: ${VSWHERE_EXE}")
	ELSE()
		SET(VSWHERE_EXE ${CMAKE_BINARY_DIR}/tools/vswhere.exe)
		MESSAGE("-- Downloading vswhere...")
		FILE(DOWNLOAD https://github.com/Microsoft/vswhere/releases/download/2.6.7/vswhere.exe ${VSWHERE_EXE})
		MESSAGE("vswhere.exe downloaded and saved to ${VSWHERE_EXE}")
	ENDIF()

	EXECUTE_PROCESS(
		COMMAND ${VSWHERE_EXE} -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
		OUTPUT_VARIABLE VS_PATH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	FIND_PROGRAM(MSBUILD_EXE msbuild PATHS "${VS_PATH}/MSBuild/15.0/Bin")

	IF(NOT MSBUILD_EXE)
		FIND_PROGRAM(MSBUILD_EXE msbuild PATHS "${VS_PATH}/MSBuild/Current/Bin")
	ENDIF()

	IF(NOT MSBUILD_EXE)
		MESSAGE(FATAL_ERROR "msbuild not found.")
	ENDIF()

	MESSAGE("-- Visual Studio path: ${VS_PATH}")
	MESSAGE("-- MSBuild: ${MSBUILD_EXE}")

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(PLATFORM x64)
    else()
		set(PLATFORM Win32)
    endif()

	string(REGEX MATCH "[0-9][0-9][0-9][0-9]" VS_VERSION "${VS_PATH}")
	message("VS_VERSION: ${VS_VERSION}")
    if ("${VS_VERSION}" STREQUAL "2017")
        # Visual Studio 15 2017
         set(CMAKE_GENERATOR_TOOLSET "v141")
        set(CMAKE_VS_PLATFORM_TOOLSET "v141")
    elseif ("${VS_VERSION}" EQUAL "2019")
        # Visual Studio 16 2019
        set(CMAKE_GENERATOR_TOOLSET "v142")
        set(CMAKE_VS_PLATFORM_TOOLSET "v142")
    else()
		message("Found unsupported Visual Studio version (${VS_VERSION}), defaulting to v142 toolset version")
        set(CMAKE_GENERATOR_TOOLSET "v142")
        set(CMAKE_VS_PLATFORM_TOOLSET "v142")
    endif()

	message("VS Platform Toolset: ${CMAKE_VS_PLATFORM_TOOLSET}")

	message("external lib include: ${ravendb_cpp_client_SOURCE_DIR}/external")

	include(FindWindowsSDK.cmake)
	message("WinSDK: ${WINDOWSSDK_LATEST_DIR}")

	if(NOT WINDOWSSDK_FOUND)
		message(FATAL_ERROR "Didn't find any Windows SDKs, cannot continue CMake generation.")
	else()
		get_windowssdk_include_dirs(${WINDOWSSDK_LATEST_DIR} INCLUDE_DIRS)		
	endif()

	message("Windows SDK include folders: ${INCLUDE_DIRS}")
	find_program(CppCompiler cl)
	get_filename_component(CppCompilerPath "${CppCompiler}" DIRECTORY)
	

	get_filename_component(VC_BUILD_TOOLS_INCLUDE_DIR "${CppCompilerPath}/../../../Include" ABSOLUTE)
	message("Build tools include path: ${VC_BUILD_TOOLS_INCLUDE_DIR}")	

	set(ENV{INCLUDE} "${ravendb_cpp_client_SOURCE_DIR}/external;${ravendb_cpp_client_SOURCE_DIR}/Raven.CppClient;${INCLUDE_DIRS};${VC_BUILD_TOOLS_INCLUDE_DIR};${CMAKE_INCLUDE_PATH}")
	execute_process(COMMAND 
		${MSBUILD_EXE} 
		/m
		/p:OutDir=${CPP_CLIENT_SRC}/binary/
		/p:IntDir=${CPP_CLIENT_SRC}/binary/obj/
	    /p:Configuration=Release
        /p:Platform=${PLATFORM}
		/p:UseEnv=true
		/p:WindowsTargetPlatformVersion=10.0
		/p:PlatformToolset=${CMAKE_VS_PLATFORM_TOOLSET}
		${ravendb_cpp_client_SOURCE_DIR}/Raven.CppClient/Raven.CppClient.vcxproj
		WORKING_DIRECTORY ${CPP_CLIENT_SRC})		
else()
	message(FATAL_ERROR "Not supported on non-Windows platforms (TODO: add proper support)")
endif()