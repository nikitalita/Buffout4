#pragma once

namespace Fixes
{
	class UnalignedLoadFix
	{
	public:
		static void Install()
		{
			REL::Relocation<std::uintptr_t> target{ REL::ID(44611), 0x172 + 0x2 };
			REL::safe_write(target.address(), std::uint32_t{ 0x10 });
			logger::info("installed {}"sv, typeid(UnalignedLoadFix).name());
		}
	};
}
