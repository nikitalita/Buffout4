#pragma once

namespace Fixes
{
	class UnalignedLoadFix
	{
	public:
		static void Install()
		{
			ApplySkinningToGeometry();
			CreateCommandBuffer();
			logger::info("installed {}"sv, typeid(UnalignedLoadFix).name());
		}

	private:
		static void ApplySkinningToGeometry()
		{
			REL::Relocation<std::uintptr_t> target{ REL::ID(44611), 0x172 + 0x2 };
			REL::safe_write(target.address(), std::uint32_t{ 0x10 });
		}

		static void CreateCommandBuffer()
		{
			constexpr std::array offsets{
				0x320,
				0x339,
				0x341 + 0x1,  // rex prefix
				0x353,
			};

			REL::Relocation<std::uintptr_t> base{ REL::ID(768994) };
			for (const auto off : offsets) {
				REL::safe_write(base.address() + off + 0x1, std::uint8_t{ 0x11 });	// movaps -> movups
			}
		}
	};
}
