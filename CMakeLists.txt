cmake_minimum_required(VERSION 3.21)

option(COPY_BUILD "Copy the build output to the Fallout 4 directory." OFF)

macro(set_from_environment VARIABLE)
	if(NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
		set(${VARIABLE} $ENV{${VARIABLE}})
	endif()
endmacro()

set_from_environment(Fallout4Path)

# project configuration

project(
	Buffout4
	VERSION 1.23.1
	LANGUAGES CXX
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/Version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	@ONLY
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.rc.in
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	@ONLY
)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(FATAL_ERROR "in-source builds are not allowed")
endif()

# dependencies

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
set(Boost_USE_STATIC_LIBS ON)

add_subdirectory("extern/CommonLibF4/CommonLibF4" CommonLibF4)

find_package(AutoTOML REQUIRED CONFIG)
find_package(Boost
	MODULE
	REQUIRED
	COMPONENTS
		nowide
		stacktrace_windbg
)
find_package(fmt REQUIRED CONFIG)
find_package(frozen REQUIRED CONFIG)
find_package(infoware REQUIRED CONFIG)
find_package(robin_hood REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)
find_package(TBB
	REQUIRED
	COMPONENTS
		tbb
		tbbmalloc
	CONFIG
)
find_package(xbyak REQUIRED CONFIG)

# create library

include("cmake/sourcelist.cmake")

source_group(
	TREE "${CMAKE_CURRENT_SOURCE_DIR}"
	FILES ${SOURCES}
)

source_group(
	TREE "${CMAKE_CURRENT_BINARY_DIR}"
	FILES "${CMAKE_CURRENT_BINARY_DIR}/include/Version.h"
)

add_library(
	"${PROJECT_NAME}"
	SHARED
	${SOURCES}
	"${CMAKE_CURRENT_BINARY_DIR}/include/Version.h"
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
	res/Buffout4.toml
	.clang-format
	.editorconfig
)

target_compile_definitions(
	"${PROJECT_NAME}"
	PRIVATE
		BOOST_STACKTRACE_LINK
		BOOST_STACKTRACE_USE_WINDBG
)

target_compile_definitions(
	CommonLibF4
	PUBLIC
		F4SE_SUPPORT_XBYAK
)

target_compile_features(
	"${PROJECT_NAME}"
	PRIVATE
		cxx_std_20
)

target_include_directories(
	"${PROJECT_NAME}"
	PRIVATE
		"${CMAKE_CURRENT_BINARY_DIR}/include"
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(
	"${PROJECT_NAME}"
	PRIVATE
		AutoTOML::AutoTOML
		Boost::headers
		Boost::nowide
		Boost::stacktrace_windbg
		Bcrypt.lib
		CommonLibF4::CommonLibF4
		Dbghelp.lib
		fmt::fmt
		frozen::frozen
		infoware
		robin_hood::robin_hood
		spdlog::spdlog
		TBB::tbb
		TBB::tbbmalloc
		tomlplusplus::tomlplusplus
		xbyak::xbyak
)

target_precompile_headers(
	"${PROJECT_NAME}"
	PRIVATE
		"src/PCH.h"
)

if(MSVC)
	target_compile_options(
		"${PROJECT_NAME}"
		PRIVATE
			"/EHa"	# Exception handling model
			"/sdl"	# Enable Additional Security Checks
			"/utf-8"	# Set Source and Executable character sets to UTF-8
			"/Zi"	# Debug Information Format

			"/permissive-"	# Standards conformance
			"/Zc:preprocessor"	# Enable preprocessor conformance mode

			# disable warnings
			"/wd4324" # 'struct_name' : structure was padded due to __declspec(align())

			"$<$<CONFIG:DEBUG>:>"
			"$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		"${PROJECT_NAME}"
		PRIVATE
			/WX	# Treat Linker Warnings as Errors

			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif()

# copy build?

if(COPY_BUILD)
	if(DEFINED Fallout4Path)
		add_custom_command(
			TARGET "${PROJECT_NAME}"
			POST_BUILD
			COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${Fallout4Path}/Data/F4SE/Plugins/"
			COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${Fallout4Path}/Data/F4SE/Plugins/"
			COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/res/${PROJECT_NAME}.toml" "${Fallout4Path}/Data/F4SE/Plugins/"
			COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/res/${PROJECT_NAME}_preload.txt" "${Fallout4Path}/Data/F4SE/Plugins/"
		)
	else()
		message(
			WARNING
			"Variable Fallout4Path is not defined. Skipping post-build copy command."
		)
	endif()
endif()

# package result

set(Python3_FIND_STRATEGY VERSION)
find_package(Python3
    MODULE
    REQUIRED
    COMPONENTS
        Interpreter
)

if(Python3_FOUND)
    set(SCRIPT "scripts/archive_artifacts.py")

    add_custom_target(
        create_artifact
        COMMAND
            "$<TARGET_FILE:Python3::Interpreter>"
            "${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT}"
            "--src-dir=${CMAKE_CURRENT_SOURCE_DIR}"
            "--name=${PROJECT_NAME}"
            "--dll=$<TARGET_FILE:${PROJECT_NAME}>"
            "--pdb=$<TARGET_PDB_FILE:${PROJECT_NAME}>"
        WORKING_DIRECTORY
            "${CMAKE_CURRENT_BINARY_DIR}"
        VERBATIM
        SOURCES
            "${SCRIPT}"
    )

    add_dependencies(
        create_artifact
        "${PROJECT_NAME}"
    )

    source_group(
        TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES "${SCRIPT}"
    )
else()
    message(WARNING "failed to find python: skipping artifact target")
endif()
