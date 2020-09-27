#pragma once

namespace Compatibility
{
	class ClassicHolsteredWeapons
	{
	public:
		static void Install()
		{
			const auto handle = static_cast<const std::byte*>(WinAPI::GetModuleHandle(L"ClassicHolsteredWeapons.dll"));
			if (handle != nullptr) {
				{
					constexpr std::size_t size = 0x64;
					const auto dst = reinterpret_cast<std::uintptr_t>(handle) + 0x0009BB0;
					REL::safe_fill(dst, REL::INT3, size);
					stl::asm_jump(dst, size, reinterpret_cast<std::uintptr_t>(&Create));
				}

				logger::info("installed {}"sv, typeid(ClassicHolsteredWeapons).name());
			} else {
				logger::warn("failed to install {}"sv, typeid(ClassicHolsteredWeapons).name());
			}
		}

	private:
		static RE::NiNode* Create(std::uint16_t a_numChildren)
		{
			return new RE::NiNode(a_numChildren);
		}
	};
}
