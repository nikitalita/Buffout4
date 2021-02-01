set(headers ${headers}
	src/Hash.h
	src/PCH.h
	src/Settings.h
	src/Compatibility/Compatibility.h
	src/Compatibility/F4EE.h
	src/Crash/CrashHandler.h
	src/Crash/Introspection/Introspection.h
	src/Crash/Modules/ModuleHandler.h
	src/Fixes/ActorIsHostileToActorFix.h
	src/Fixes/CellInitFix.h
	src/Fixes/EncounterZoneResetFix.h
	src/Fixes/Fixes.h
	src/Fixes/GreyMoviesFix.h
	src/Fixes/MovementPlannerFix.h
	src/Fixes/PackageAllocateLocationFix.h
	src/Fixes/SafeExitFix.h
	src/Fixes/UnalignedLoadFix.h
	src/Fixes/UtilityShaderFix.h
	src/Patches/AchievementsPatch.h
	src/Patches/BSTextureStreamerLocalHeapPatch.h
	src/Patches/HavokMemorySystemPatch.h
	src/Patches/MaxStdIOPatch.h
	src/Patches/MemoryManagerPatch.h
	src/Patches/Patches.h
	src/Patches/ScaleformAllocatorPatch.h
	src/Patches/SmallBlockAllocatorPatch.h
	src/Patches/WorkshopMenuPatch.h
	src/Warnings/CreateTexture2DWarning.h
	src/Warnings/ImageSpaceAdapterWarning.h
	src/Warnings/Warnings.h
)
