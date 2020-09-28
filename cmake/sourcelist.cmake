set(sources ${sources}
	src/main.cpp
	src/PCH.cpp
	src/Compatibility/Compatibility.cpp
	src/Crash/CrashHandler.cpp
	src/Crash/Introspection/Introspection.cpp
	src/Crash/Modules/ModuleHandler.cpp
	src/Fixes/CellInitFix.cpp
	src/Fixes/Fixes.cpp
	src/Patches/MemoryManagerPatch.cpp
	src/Patches/Patches.cpp
	src/Patches/SmallBlockAllocatorPatch.cpp
	src/Warnings/ImageSpaceAdapterWarning.cpp
	src/Warnings/Warnings.cpp
)
