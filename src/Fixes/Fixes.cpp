#include "Fixes/Fixes.h"

#include "Fixes/ActorIsHostileToActorFix.h"
#include "Fixes/CellInitFix.h"
#include "Fixes/EncounterZoneResetFix.h"
#include "Fixes/FaderMenuFix.h"
#include "Fixes/SafeExit.h"
#include "Fixes/UnalignedLoadFix.h"

namespace Fixes
{
	void PreInit()
	{
		Settings::load();

		if (*Settings::ActorIsHostileToActor) {
			ActorIsHostileToActorFix::Install();
		}

		if (*Settings::CellInit) {
			CellInitFix::Install();
		}

		if (*Settings::FaderMenu) {
			FaderMenuFix::Install();
		}

		if (*Settings::SafeExit) {
			SafeExit::Install();
		}

		if (*Settings::UnalignedLoad) {
			UnalignedLoadFix::Install();
		}
	}

	void PostInit()
	{
		if (*Settings::EncounterZoneReset) {
			EncounterZoneResetFix::Install();
		}
	}
}
