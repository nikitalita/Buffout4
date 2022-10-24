#pragma once

namespace Fixes::MovementPlannerFix
{
	namespace detail
	{
		struct CanWarpOnPathFailure
		{
			static bool thunk(const RE::Actor* a_actor)
			{
				return a_actor ? func(a_actor) : true;
			}

			static inline REL::Relocation<decltype(thunk)> func;
		};
	}

	inline void Install()
	{
#ifndef FALLOUTVR
		REL::Relocation<std::uintptr_t> target{ REL::ID(1403049), 0x30 };
#else
		REL::Relocation<std::uintptr_t> target{ REL::Offset(0xfba2c0).address() + 0x30};
#endif  // !FALLOUTVR

		stl::write_thunk_call<5, detail::CanWarpOnPathFailure>(target.address());
		logger::debug("installed MovementPlanner fix"sv);
	}
}
