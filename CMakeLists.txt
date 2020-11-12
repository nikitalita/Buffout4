cmake_minimum_required(VERSION 3.18)

# ---- Cache build vars ----

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(VCPKG_OVERLAY_TRIPLETS "triplets" CACHE STRING "")
set(VCPKG_TARGET_TRIPLET "x64-windows-static-custom" CACHE STRING "")

set(MY_CXX_FLAGS "")
string(
	APPEND
	MY_CXX_FLAGS
)
string(STRIP "${MY_CXX_FLAGS}" MY_CXX_FLAGS)
set(VCPKG_CXX_FLAGS "${MY_CXX_FLAGS}" CACHE STRING "" FORCE)

set(MY_CXX_FLAGS_DEBUG "")
string(
	APPEND
	MY_CXX_FLAGS_DEBUG
	
	" /JMC "	# Just My Code debugging
	" /Zc:inline- "	# Keep unreferenced COMDAT
)
string(STRIP "${MY_CXX_FLAGS_DEBUG}" MY_CXX_FLAGS_DEBUG)
set(VCPKG_CXX_FLAGS_DEBUG "${MY_CXX_FLAGS_DEBUG}" CACHE STRING "" FORCE)

set(MY_CXX_FLAGS_RELEASE "")
string(
	APPEND
	MY_CXX_FLAGS_RELEASE
	
	" /JMC- "	# Disable Just My Code debugging
	" /Ob3 "	# Inline Function Expansion
	" /Zc:inline "	# Remove unreferenced COMDAT
)
string(STRIP "${MY_CXX_FLAGS_RELEASE}" MY_CXX_FLAGS_RELEASE)
set(VCPKG_CXX_FLAGS_RELEASE "${MY_CXX_FLAGS_RELEASE}" CACHE STRING "" FORCE)

set(MY_LINKER_FLAGS_DEBUG "")
string(
	APPEND
	MY_LINKER_FLAGS_DEBUG
	
	" /INCREMENTAL "	# Link Incrementally
	" /OPT:NOICF "	# Optimizations (prevent identical COMDAT folding)
	" /OPT:NOREF "	# Optimizations (keep functions/data never referenced)
)
string(STRIP "${MY_LINKER_FLAGS_DEBUG}" MY_LINKER_FLAGS_DEBUG)
set(VCPKG_LINKER_FLAGS_DEBUG "${MY_LINKER_FLAGS_DEBUG}" CACHE STRING "" FORCE)

set(MY_LINKER_FLAGS_RELEASE "")
string(
	APPEND
	MY_LINKER_FLAGS_RELEASE
	
	" /INCREMENTAL:NO "	# Link Incrementally
	" /OPT:ICF "	# Optimizations (perform identical COMDAT folding)
	" /OPT:REF "	# Optimizations (eliminate functions/data never referenced)
)
string(STRIP "${MY_LINKER_FLAGS_RELEASE}" MY_LINKER_FLAGS_RELEASE)
set(VCPKG_LINKER_FLAGS_RELEASE "${MY_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)

# ---- Project ----

project(
	Buffout4
	VERSION 1.17.1
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

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
	message(
		FATAL_ERROR
			"In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
)
endif()

# ---- Globals ----

add_compile_definitions(
	"$<$<BOOL:${MSVC}>:_UNICODE>"
	BOOST_STACKTRACE_LINK
	BOOST_STACKTRACE_USE_WINDBG
	F4SE_SUPPORT_XBYAK
	TOML_WINDOWS_COMPAT=0
)

if (MSVC)
	string(REPLACE " " ";" MY_CXX_FLAGS "${VCPKG_CXX_FLAGS}")
	string(REPLACE " " ";" MY_CXX_FLAGS_DEBUG "${VCPKG_CXX_FLAGS_DEBUG}")
	string(REPLACE " " ";" MY_CXX_FLAGS_RELEASE "${VCPKG_CXX_FLAGS_RELEASE}")
	
	add_compile_options(
		/EHa	# Enables standard C++ stack unwinding. Catches synchronous/asynchronous with catch(...) syntax
		/MP	# Build with Multiple Processes
		/sdl	# Enable Additional Security Checks
		/Zi	# Debug Information Format
		
		/permissive-	# Standards conformance
		
		/Zc:alignedNew	# C++17 over-aligned allocation
		/Zc:auto	# Deduce Variable Type
		/Zc:char8_t
		/Zc:__cplusplus	# Enable updated __cplusplus macro
		/Zc:externC
		/Zc:externConstexpr	# Enable extern constexpr variables
		/Zc:forScope	# Force Conformance in for Loop Scope
		/Zc:hiddenFriend
		/Zc:implicitNoexcept	# Implicit Exception Specifiers
		/Zc:lambda
		/Zc:noexceptTypes	# C++17 noexcept rules
		/Zc:preprocessor	# Enable preprocessor conformance mode
		/Zc:referenceBinding	# Enforce reference binding rules
		/Zc:rvalueCast	# Enforce type conversion rules
		/Zc:sizedDealloc	# Enable Global Sized Deallocation Functions
		/Zc:strictStrings	# Disable string literal type conversion
		/Zc:ternary	# Enforce conditional operator rules
		/Zc:threadSafeInit	# Thread-safe Local Static Initialization
		/Zc:tlsGuards
		/Zc:trigraphs	# Trigraphs Substitution
		/Zc:wchar_t	# wchar_t Is Native Type
		
		/experimental:external
		/external:anglebrackets
		/external:W0
		
		/W4	# Warning level
		/WX	# Warning level (warnings are errors)
		
		${MY_CXX_FLAGS}
		"$<$<CONFIG:DEBUG>:${MY_CXX_FLAGS_DEBUG}>"
		"$<$<CONFIG:RELEASE>:${MY_CXX_FLAGS_RELEASE}>"
	)
	
	# fix a warning with /Ob2 being overridden with /Ob3
	string(REPLACE "/Ob2" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

# ---- Dependencies ----

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_DEBUG_RUNTIME ON)

add_subdirectory($ENV{CommonLibF4Path} CommonLibF4)

find_package(Boost
	REQUIRED
	COMPONENTS
		stacktrace_windbg
)
find_package(fmt REQUIRED CONFIG)
find_package(frozen REQUIRED CONFIG)
find_package(infoware REQUIRED CONFIG)
find_package(spdlog REQUIRED CONFIG)
find_package(TBB
	REQUIRED
	COMPONENTS
		tbbmalloc
	CONFIG
)
find_package(tomlplusplus REQUIRED CONFIG)

find_path(ROBIN_HOOD_HASHING_INCLUDE_DIRS "robin_hood.h")
find_path(XBYAK_INCLUDE_DIRS "xbyak/xbyak.h")

# ---- Add source files ----

include(cmake/headerlist.cmake)
include(cmake/sourcelist.cmake)

source_group(
	TREE
		${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		${headers}
		${sources}
)

source_group(
	TREE
		${CMAKE_CURRENT_BINARY_DIR}
	FILES
		${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
)

# ---- Create DLL ----

add_library(
	${PROJECT_NAME}
	SHARED
	${headers}
	${sources}
	${CMAKE_CURRENT_BINARY_DIR}/include/Version.h
	${CMAKE_CURRENT_BINARY_DIR}/version.rc
	.clang-format
	Buffout4.toml
)

set_property(
	TARGET
		${PROJECT_NAME}
		CommonLibF4
	PROPERTY
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

target_compile_features(
	${PROJECT_NAME}
	PUBLIC
		cxx_std_17
)

if (MSVC)
	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
			/TP	# Specify Source File Type (C++)
			
			# disable warnings
			/wd4324 # 'struct_name' : structure was padded due to __declspec(align())
	)
endif()

target_include_directories(
	${PROJECT_NAME}
	PRIVATE 
		${CMAKE_CURRENT_BINARY_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${ROBIN_HOOD_HASHING_INCLUDE_DIRS}
		${XBYAK_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC
		Boost::headers
		Boost::stacktrace_windbg
		CommonLibF4
		Dbghelp.lib
		fmt::fmt
		frozen::frozen
		infoware
		spdlog::spdlog
		TBB::tbbmalloc
		tomlplusplus::tomlplusplus
)

if (MSVC)
	string(REPLACE " " ";" MY_LINKER_FLAGS_DEBUG "${VCPKG_LINKER_FLAGS_DEBUG}")
	string(REPLACE " " ";" MY_LINKER_FLAGS_RELEASE "${VCPKG_LINKER_FLAGS_RELEASE}")
	
	target_link_options(
		${PROJECT_NAME}
		PRIVATE
			"$<$<CONFIG:DEBUG>:${MY_LINKER_FLAGS_DEBUG}>"
			"$<$<CONFIG:RELEASE>:${MY_LINKER_FLAGS_RELEASE};/DEBUG:FULL>"
	)
endif()

target_precompile_headers(
	${PROJECT_NAME}
	PRIVATE
		src/PCH.h
)

# ---- Post build ----

add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> $ENV{Fallout4Path}/Data/F4SE/Plugins/
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> $ENV{Fallout4Path}/Data/F4SE/Plugins/
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.toml $ENV{Fallout4Path}/Data/F4SE/Plugins/
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}_preload.txt $ENV{Fallout4Path}/Data/F4SE/Plugins/
)
