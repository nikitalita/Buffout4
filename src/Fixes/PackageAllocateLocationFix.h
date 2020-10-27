#pragma once

namespace Fixes
{
	class PackageAllocateLocationFix
	{
	public:
		static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ REL::ID(1248203), 0x141 };
			auto& trampoline = F4SE::GetTrampoline();
			_GetPrimitive = trampoline.write_call<5>(target.address(), GetPrimitive);
			logger::info("installed {}"sv, typeid(PackageAllocateLocationFix).name());
		}

	private:
		static RE::BGSPrimitive* GetPrimitive(const RE::ExtraDataList* a_this)
		{
			return a_this ? _GetPrimitive(a_this) : nullptr;
		}

		static inline REL::Relocation<decltype(GetPrimitive)> _GetPrimitive;
	};
}
