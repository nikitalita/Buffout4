#pragma once

namespace TESObjectREFRGetEncounterZoneFix
{
	namespace detail
	{
		template <class T>
		struct GetEncounterZone
		{
			static T* thunk(const RE::TESObjectREFR& a_this)
			{
				auto ptr = a_this.extraList ? func(*a_this.extraList) : nullptr;

				const auto addr = reinterpret_cast<std::uintptr_t>(ptr);
				if (!a_this.IsInitialized() &&
					((addr & 0xFFFF'FFFF'0000'0000) == 0) &&
					((addr & 0x0000'0000'FFFF'FFFF) != 0)) {
					auto id = static_cast<std::uint32_t>(addr);
					RE::TESForm::AddCompileIndex(id, a_this.GetFile());
					ptr = RE::TESForm::GetFormByID<T>(id);
				}

				return ptr;
			}

			static inline REL::Relocation<T*(const RE::ExtraDataList&)> func;
		};
	}

	inline void Install()
	{
		const auto root = REL::ID(1413642).address();

		REL::safe_fill(root + 0xD, REL::NOP, 0x7);
		stl::write_thunk_call<5, detail::GetEncounterZone<RE::BGSEncounterZone>>(root + 0x14);

		REL::safe_fill(root + 0x91, REL::NOP, 0x7);
		stl::write_thunk_call<5, detail::GetEncounterZone<RE::BGSLocation>>(root + 0x98);

		logger::debug("installed TESObjectREFRGetEncounterZone fix"sv);
	}
}
