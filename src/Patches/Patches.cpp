#include "Patches/Patches.h"

#include "Patches/AchievementsPatch.h"
#include "Patches/BSMTAManagerPatch.h"
#include "Patches/BSPreCulledObjectsPatch.h"
#include "Patches/BSTextureStreamerLocalHeapPatch.h"
#include "Patches/HavokMemorySystemPatch.h"
#include "Patches/INISettingCollectionPatch.h"
#include "Patches/InputSwitchPatch.h"
#include "Patches/MaxStdIOPatch.h"
#include "Patches/MemoryManagerPatch.h"
#include "Patches/ScaleformAllocatorPatch.h"
#include "Patches/SmallBlockAllocatorPatch.h"
#include "Patches/WorkshopMenuPatch.h"

namespace Patches
{
	void PreLoad()
	{
		if (*Settings::Achievements) {
			AchievementsPatch::Install();
		}

#ifndef FALLOUTVR
		// partial work on this one.     BSMTAManager::RegisterObjects::execute function missing from vr so need to rethink this one
		if (*Settings::BSMTAManager) {
			BSMTAManagerPatch::Install();
		}

		if (*Settings::BSPreCulledObjects) {
			BSPreCulledObjectsPatch::Install();
		}

#endif
		if (*Settings::BSTextureStreamerLocalHeap) {
			BSTextureStreamerLocalHeapPatch::Install();
		}

		if (*Settings::HavokMemorySystem) {
			HavokMemorySystemPatch::Install();
		}

		if (*Settings::INISettingCollection) {
			INISettingCollectionPatch::Install();
		}

#ifndef FALLOUTVR
		if (*Settings::InputSwitch) {
			InputSwitchPatch::PreLoad();
		}
#endif

		if (*Settings::MaxStdIO != -1) {
			MaxStdIOPatch::Install();
		}

		if (*Settings::MemoryManager || *Settings::MemoryManagerDebug) {
			MemoryManagerPatch::Install();
		}

		if (*Settings::ScaleformAllocator) {
			ScaleformAllocatorPatch::Install();
		}

		if (*Settings::SmallBlockAllocator) {
			SmallBlockAllocatorPatch::Install();
		}

#ifndef FALLOUTVR
		if (*Settings::WorkshopMenu) {
			WorkshopMenuPatch::Install();
		}
#endif
	}

	void PostInit()
	{
		if (*Settings::InputSwitch) {
			InputSwitchPatch::PostInit();
		}
	}
}
