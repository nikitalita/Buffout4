#pragma once

namespace Warnings
{
	class ImageSpaceAdapterWarning
	{
	public:
		static void Install()
		{
			auto& trampoline = F4SE::GetTrampoline();
			REL::Relocation<std::uintptr_t> target{ REL::ID(231868), 0x359 };
			_original = trampoline.write_call<5>(target.address(), GetChunkData);
			logger::info("installed {}"sv, typeid(ImageSpaceAdapterWarning).name());
		}

	private:
		static bool GetChunkData(RE::TESFile* a_this, void* a_data, std::uint32_t a_maxSize)
		{
			const auto result = _original(a_this, a_data, a_maxSize);
			const auto imad = stl::adjust_pointer<RE::TESImageSpaceModifier>(a_data, -0x20);
			for (auto& sizes : imad->data.keySize) {
				if (sizes[0] == 0 || sizes[1] == 0) {
					stl::report_and_fail(
						fmt::format(
							"[IMAD 0x{:08X}] from \"{}\" has an invalid key size of zero. "
							"This will result in memory corruption. "
							"Please open the form in xedit and correct it or remove the mod from your load order."sv,
							imad->GetFormID(),
							a_this->GetFilename()));
				}
			}
			return result;
		}

		static inline REL::Relocation<decltype(GetChunkData)> _original;
	};
}
