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
				SetMorphValues(base);
				UpdateOverlays(base);
				logger::info("installed {}"sv, typeid(F4EE).name());
			} else {
				logger::warn("failed to install {}"sv, typeid(F4EE).name());
			}
		}

	private:
		static void* AllocateNiNode()
		{
			return RE::aligned_alloc<RE::NiNode>();
		}

		static void CopyMorphs(const float a_src[], std::size_t a_size, RE::BSTArray<float>& a_dst)
		{
			constexpr std::size_t max = 5;
			a_dst.resize(max);
			std::fill(a_dst.begin(), a_dst.end(), 0.0F);
			for (std::size_t i = 0; i < std::min(a_size, max); ++i) {
				a_dst[static_cast<std::uint32_t>(i)] = a_src[i];
			}
		}

		static RE::BSTArray<float>* CreateMorphs()
		{
			return new RE::BSTArray<float>(5, 0.0F);
		}

		static RE::NiNode* CreateNiNode()
		{
			return new RE::NiNode();
		}

		static void GetOverlayRoot(std::uintptr_t a_base)
		{
			constexpr std::size_t size = 0x51;
			const auto dst = a_base + 0x0001F40;
			REL::safe_fill(dst, REL::INT3, size);
			stl::asm_jump(dst, size, reinterpret_cast<std::uintptr_t>(&CreateNiNode));
		}

		static void SimpleInlinePatch(std::uintptr_t a_dst, std::size_t a_size, std::uintptr_t a_func);
		static void SetMorphValues(std::uintptr_t a_base);

		static void UpdateOverlays(std::uintptr_t a_base)
		{
			constexpr std::size_t first = 0x0043416;
			constexpr std::size_t last = 0x004343E;
			constexpr std::size_t size = last - first;
			const auto dst = a_base + first;
			SimpleInlinePatch(dst, size, reinterpret_cast<std::uintptr_t>(&AllocateNiNode));
		}
	};
}
