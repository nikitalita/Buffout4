cmake_minimum_required(VERSION 3.23)

project(
	Buffout4
	VERSION 1.26.2
	LANGUAGES CXX
)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(FATAL_ERROR "in-source builds are not allowed")
endif()

include(cmake/common.cmake)

set_from_environment(Fallout4Path)
if(NOT DEFINED Fallout4Path)
	message(FATAL_ERROR "Fallout4Path is not set")
endif()

add_subdirectory(src)
include(cmake/packaging.cmake)
