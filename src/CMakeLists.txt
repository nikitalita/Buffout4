set_root_directory()

set(SOURCE_DIR "${ROOT_DIR}/src")
set(SOURCE_FILES
	"${SOURCE_DIR}/Compatibility/Compatibility.cpp"
	"${SOURCE_DIR}/Compatibility/Compatibility.h"
	"${SOURCE_DIR}/Compatibility/F4EE.cpp"
	"${SOURCE_DIR}/Compatibility/F4EE.h"
	"${SOURCE_DIR}/Crash/Introspection/Introspection.cpp"
	"${SOURCE_DIR}/Crash/Introspection/Introspection.h"
	"${SOURCE_DIR}/Crash/Modules/ModuleHandler.cpp"
	"${SOURCE_DIR}/Crash/Modules/ModuleHandler.h"
	"${SOURCE_DIR}/Crash/CrashHandler.cpp"
	"${SOURCE_DIR}/Crash/CrashHandler.h"
	"${SOURCE_DIR}/Fixes/ActorIsHostileToActorFix.h"
	"${SOURCE_DIR}/Fixes/CellInitFix.cpp"
	"${SOURCE_DIR}/Fixes/CellInitFix.h"
	"${SOURCE_DIR}/Fixes/CreateD3DAndSwapChainFix.cpp"
	"${SOURCE_DIR}/Fixes/CreateD3DAndSwapChainFix.h"
	"${SOURCE_DIR}/Fixes/EncounterZoneResetFix.h"
	"${SOURCE_DIR}/Fixes/Fixes.cpp"
	"${SOURCE_DIR}/Fixes/Fixes.h"
	"${SOURCE_DIR}/Fixes/GreyMoviesFix.h"
	"${SOURCE_DIR}/Fixes/MagicEffectApplyEventFix.h"
	"${SOURCE_DIR}/Fixes/MovementPlannerFix.h"
	"${SOURCE_DIR}/Fixes/PackageAllocateLocationFix.h"
	"${SOURCE_DIR}/Fixes/SafeExitFix.h"
	"${SOURCE_DIR}/Fixes/UnalignedLoadFix.h"
	"${SOURCE_DIR}/Fixes/UtilityShaderFix.cpp"
	"${SOURCE_DIR}/Fixes/UtilityShaderFix.h"
	"${SOURCE_DIR}/Patches/AchievementsPatch.cpp"
	"${SOURCE_DIR}/Patches/AchievementsPatch.h"
	"${SOURCE_DIR}/Patches/BSMTAManagerPatch.h"
	"${SOURCE_DIR}/Patches/BSPreCulledObjectsPatch.h"
	"${SOURCE_DIR}/Patches/BSTextureStreamerLocalHeapPatch.h"
	"${SOURCE_DIR}/Patches/HavokMemorySystemPatch.h"
	"${SOURCE_DIR}/Patches/INISettingCollectionPatch.h"
	"${SOURCE_DIR}/Patches/InputSwitchPatch.cpp"
	"${SOURCE_DIR}/Patches/InputSwitchPatch.h"
	"${SOURCE_DIR}/Patches/MaxStdIOPatch.h"
	"${SOURCE_DIR}/Patches/MemoryManagerPatch.cpp"
	"${SOURCE_DIR}/Patches/MemoryManagerPatch.h"
	"${SOURCE_DIR}/Patches/Patches.cpp"
	"${SOURCE_DIR}/Patches/Patches.h"
	"${SOURCE_DIR}/Patches/ScaleformAllocatorPatch.h"
	"${SOURCE_DIR}/Patches/SmallBlockAllocatorPatch.cpp"
	"${SOURCE_DIR}/Patches/SmallBlockAllocatorPatch.h"
	"${SOURCE_DIR}/Patches/WorkshopMenuPatch.h"
	"${SOURCE_DIR}/Warnings/CreateTexture2DWarning.cpp"
	"${SOURCE_DIR}/Warnings/CreateTexture2DWarning.h"
	"${SOURCE_DIR}/Warnings/ImageSpaceAdapterWarning.cpp"
	"${SOURCE_DIR}/Warnings/ImageSpaceAdapterWarning.h"
	"${SOURCE_DIR}/Warnings/Warnings.cpp"
	"${SOURCE_DIR}/Warnings/Warnings.h"
	"${SOURCE_DIR}/Allocator.cpp"
	"${SOURCE_DIR}/Allocator.h"
	"${SOURCE_DIR}/Hash.cpp"
	"${SOURCE_DIR}/Hash.h"
	"${SOURCE_DIR}/main.cpp"
	"${SOURCE_DIR}/PCH.cpp"
	"${SOURCE_DIR}/PCH.h"
	"${SOURCE_DIR}/Settings.h"
)

source_group(TREE "${ROOT_DIR}" FILES ${SOURCE_FILES})

set(VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/src/Plugin.h")
configure_file(
	"${ROOT_DIR}/cmake/Plugin.h.in"
	"${VERSION_HEADER}"
	@ONLY
)

source_group("src" FILES "${VERSION_HEADER}")

configure_file(
	"${ROOT_DIR}/cmake/version.rc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
	@ONLY
)

add_library(
	"${PROJECT_NAME}"
	SHARED
	${SOURCE_FILES}
	"${VERSION_HEADER}"
	"${CMAKE_CURRENT_BINARY_DIR}/version.rc"
	"${ROOT_DIR}/.clang-format"
	"${ROOT_DIR}/.editorconfig"
)

target_compile_features(
	"${PROJECT_NAME}"
	PRIVATE
		cxx_std_23
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	target_compile_options(
		"${PROJECT_NAME}"
		PRIVATE
			"/EHa"	# Exception handling model
			"/sdl"	# Enable Additional Security Checks
			"/utf-8"	# Set Source and Executable character sets to UTF-8
			"/Zi"	# Debug Information Format

			"/permissive-"	# Standards conformance
			"/Zc:preprocessor"	# Enable preprocessor conformance mode

			"/wd4324" # 'struct_name' : structure was padded due to __declspec(align())

			"$<$<CONFIG:DEBUG>:>"
			"$<$<CONFIG:RELEASE>:/Zc:inline;/JMC-;/Ob3>"
	)

	target_link_options(
		"${PROJECT_NAME}"
		PRIVATE
			"$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
			"$<$<CONFIG:RELEASE>:/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
	)
endif()

target_include_directories(
	"${PROJECT_NAME}"
	PRIVATE
		"${CMAKE_CURRENT_BINARY_DIR}/src"
		"${SOURCE_DIR}"
)

add_subdirectory("${ROOT_DIR}/external/CommonLibF4/CommonLibF4" CommonLibF4 EXCLUDE_FROM_ALL)

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
find_package(tomlplusplus REQUIRED CONFIG)
find_package(xbyak REQUIRED CONFIG)

target_link_libraries(
	"${PROJECT_NAME}"
	PRIVATE
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

target_precompile_headers(
	"${PROJECT_NAME}"
	PRIVATE
		"${SOURCE_DIR}/PCH.h"
)

handle_data_files(
	FILES
		"$<TARGET_FILE:${PROJECT_NAME}>"
		"${ROOT_DIR}/data/Data/F4SE/Plugins/Buffout4_preload.txt"
		"${ROOT_DIR}/data/Data/F4SE/Plugins/Buffout4.toml"
	DESTINATION "F4SE/Plugins"
)

install(
	FILES
		"$<TARGET_PDB_FILE:${PROJECT_NAME}>"
	DESTINATION "/"
	COMPONENT "pdbs"
)

if("${COPY_OUTPUT}")
	add_custom_command(
		TARGET "${PROJECT_NAME}"
		POST_BUILD
		COMMAND
			"${CMAKE_COMMAND}"
			-E
			copy_if_different
			"$<TARGET_PDB_FILE:${PROJECT_NAME}>"
			"${Fallout4Path}/Data/F4SE/Plugins/"
		VERBATIM
	)
endif()
