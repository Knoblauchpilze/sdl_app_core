cmake_minimum_required (VERSION 3.7)

project (sdl_app_core)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library (sdl_app_core SHARED)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

target_compile_options (sdl_app_core PUBLIC
	-Wall -Wextra -Werror -pedantic
	)

set (CMAKE_VERBOSE_MAKEFILE OFF)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(
	${CMAKE_CURRENT_SOURCE_DIR}/src
	)

target_include_directories (sdl_app_core PUBLIC
	)

target_link_libraries (sdl_app_core
	core_utils
	sdl_engine
	sdl_core
	sdl_graphic
	)
