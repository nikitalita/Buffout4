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

		// partial work on this one.     BSMTAManager::RegisterObjects::execute function missing from vr so need to rethink this one
		if (REL::Module::IsF4() && *Settings::BSMTAManager) {
			BSMTAManagerPatch::Install();
		}

		if (REL::Module::IsF4() && *Settings::BSPreCulledObjects) {
			BSPreCulledObjectsPatch::Install();
		}
		if (*Settings::BSTextureStreamerLocalHeap) {
			BSTextureStreamerLocalHeapPatch::Install();
		}

		if (*Settings::HavokMemorySystem) {
			HavokMemorySystemPatch::Install();
		}

		if (*Settings::INISettingCollection) {
			INISettingCollectionPatch::Install();
		}

		if (REL::Module::IsF4() && *Settings::InputSwitch) {
			InputSwitchPatch::PreLoad();
		}
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

		if (REL::Module::IsF4() && *Settings::WorkshopMenu) {
			WorkshopMenuPatch::Install();
		}
	}

	void PostInit()
	{
		if (*Settings::InputSwitch) {
			InputSwitchPatch::PostInit();
		}
	}
}
