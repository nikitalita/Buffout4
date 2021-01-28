#pragma once

namespace Settings
{
	using ISetting = AutoTOML::ISetting;
	using bSetting = AutoTOML::bSetting;
	using iSetting = AutoTOML::iSetting;

	inline void load()
	{
		try {
			const auto table = toml::parse_file("Data/F4SE/Plugins/Buffout4.toml"s);
			for (const auto& setting : ISetting::get_settings()) {
				setting->load(table);
			}
		} catch (const toml::parse_error& e) {
			std::ostringstream ss;
			ss
				<< "Error parsing file \'" << *e.source().path << "\':\n"
				<< '\t' << e.description() << '\n'
				<< "\t\t(" << e.source().begin << ')';
			logger::error(ss.str());
			stl::report_and_fail("failed to load settings"sv);
		} catch (const std::exception& e) {
			stl::report_and_fail(e.what());
		} catch (...) {
			stl::report_and_fail("unknown failure"sv);
		}
	}

	inline bSetting ActorIsHostileToActor{ "Fixes"s, "ActorIsHostileToActor"s, true };
	inline bSetting CellInit{ "Fixes"s, "CellInit"s, true };
	inline bSetting EncounterZoneReset{ "Fixes"s, "EncounterZoneReset"s, true };
	inline bSetting GreyMovies{ "Fixes"s, "GreyMovies"s, true };
	inline bSetting MovementPlanner{ "Fixes"s, "MovementPlanner"s, true };
	inline bSetting PackageAllocateLocation{ "Fixes"s, "PackageAllocateLocation"s, true };
	inline bSetting SafeExit{ "Fixes"s, "SafeExit"s, true };
	inline bSetting UnalignedLoad{ "Fixes"s, "UnalignedLoad"s, true };
	inline bSetting UtilityShader{ "Fixes"s, "UtilityShader"s, true };

	inline bSetting Achievements{ "Patches"s, "Achievements"s, true };
	inline bSetting BSTextureStreamerLocalHeap{ "Patches"s, "BSTextureStreamerLocalHeap"s, true };
	inline bSetting HavokMemorySystem{ "Patches"s, "HavokMemorySystem"s, true };
	inline iSetting MaxStdIO{ "Patches"s, "MaxStdIO"s, -1 };
	inline bSetting MemoryManager{ "Patches"s, "MemoryManager"s, true };
	inline bSetting MemoryManagerDebug{ "Patches"s, "MemoryManagerDebug"s, false };
	inline bSetting ScaleformAllocator{ "Patches"s, "ScaleformAllocator"s, true };
	inline bSetting SmallBlockAllocator{ "Patches"s, "SmallBlockAllocator"s, true };

	inline bSetting CreateTexture2D{ "Warnings"s, "CreateTexture2D"s, true };
	inline bSetting ImageSpaceAdapter{ "Warnings"s, "ImageSpaceAdapter"s, true };

	inline bSetting F4EE{ "Compatibility"s, "F4EE"s, true };
}
