#pragma once

namespace Fixes::PackageAllocateLocationFix
{
	namespace detail
	{
		struct GetPrimitive
		{
			static RE::BGSPrimitive* thunk(const RE::ExtraDataList* a_this)
			{
				return a_this ? func(a_this) : nullptr;
			}

			static inline REL::Relocation<decltype(thunk)> func;
		};
	}

	inline void Install()
	{
#ifndef FALLOUTVR
		REL::Relocation<std::uintptr_t> target{ REL::ID(1248203), 0x141 };
#else
		REL::Relocation<std::uintptr_t> target{ REL::Offset(0x715350).address() + 0x141 };
#endif
		stl::write_thunk_call<5, detail::GetPrimitive>(target.address());
		logger::debug("installed PackageAllocateLocation fix"sv);
	}
}
