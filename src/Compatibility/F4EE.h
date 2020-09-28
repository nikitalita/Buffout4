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
				const auto base = reinterpret_cast<std::uintptr_t>(handle);
				GetOverlayRoot(base);
				UpdateOverlays(base);
				logger::info("installed {}"sv, typeid(F4EE).name());
			} else {
				logger::warn("failed to install {}"sv, typeid(F4EE).name());
			}
		}

	private:
		static void GetOverlayRoot(std::uintptr_t a_base)
		{
			constexpr std::size_t size = 0x51;
			const auto dst = a_base + 0x0001F40;
			REL::safe_fill(dst, REL::INT3, size);
			stl::asm_jump(dst, size, reinterpret_cast<std::uintptr_t>(&Create));
		}

		static void UpdateOverlays(std::uintptr_t a_base);

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
