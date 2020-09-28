#pragma once

namespace Compatibility
{
	class F4EE
	{
	public:
		static void Install()
		{
			const auto handle = static_cast<const std::byte*>(WinAPI::GetModuleHandle(L"f4ee.dll"));
			if (handle != nullptr) {
				// GetOverlayRoot
				{
					constexpr std::size_t size = 0x51;
					const auto dst = reinterpret_cast<std::uintptr_t>(handle) + 0x0001F40;
					REL::safe_fill(dst, REL::INT3, size);
					stl::asm_jump(dst, size, reinterpret_cast<std::uintptr_t>(&Create));
				}

				// UpdateOverlays
				{
					constexpr std::uintptr_t offset = 0x0043416;
					constexpr std::size_t size = 0x004343E - offset;
					const auto dst = reinterpret_cast<std::uintptr_t>(handle) + offset;
					REL::safe_fill(dst, REL::NOP, size);
					stl::asm_call(dst, size, reinterpret_cast<std::uintptr_t>(&Allocate));
				}

				logger::info("installed {}"sv, typeid(F4EE).name());
			} else {
				logger::warn("failed to install {}"sv, typeid(F4EE).name());
			}
		}

	private:
		static void* Allocate()
		{
			return RE::aligned_alloc<RE::NiNode>();
		}

		static RE::NiNode* Create()
		{
			return new RE::NiNode();
		}
	};
}
