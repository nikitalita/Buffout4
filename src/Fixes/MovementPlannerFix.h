#pragma once

namespace Fixes
{
	class MovementPlannerFix
	{
	public:
		static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ REL::ID(1403049), 0x30 };
			auto& trampoline = F4SE::GetTrampoline();
			_original = trampoline.write_call<5>(target.address(), CanWarpOnPathFailure);
			logger::info("installed {}"sv, typeid(MovementPlannerFix).name());
		}

	private:
		static bool CanWarpOnPathFailure(const RE::Actor* a_actor)
		{
			return a_actor ? _original(a_actor) : true;
		}

		static inline REL::Relocation<decltype(CanWarpOnPathFailure)> _original;
	};
}
