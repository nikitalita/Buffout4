#include "Fixes/Fixes.h"

#include "Fixes/ActorIsHostileToActorFix.h"
#include "Fixes/CellInitFix.h"
#include "Fixes/EncounterZoneResetFix.h"
#include "Fixes/GreyMoviesFix.h"
#include "Fixes/MovementPlannerFix.h"
#include "Fixes/PackageAllocateLocationFix.h"
#include "Fixes/SafeExitFix.h"
#include "Fixes/UnalignedLoadFix.h"
#include "Fixes/UtilityShaderFix.h"

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

		if (*Settings::GreyMovies) {
			GreyMoviesFix::Install();
		}

		if (*Settings::MovementPlanner) {
			MovementPlannerFix::Install();
		}

		if (*Settings::PackageAllocateLocation) {
			PackageAllocateLocationFix::Install();
		}

		if (*Settings::SafeExit) {
			SafeExitFix::Install();
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

		if (*Settings::UtilityShader) {
			UtilityShaderFix::Install();
		}
	}
}
