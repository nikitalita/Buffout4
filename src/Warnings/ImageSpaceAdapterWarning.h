#pragma once

namespace Warnings
{
	class ImageSpaceAdapterWarning
	{
	public:
		static void Install();

	private:
		static RE::NiFloatInterpolator* LoadChunk(RE::TESFile* a_file, std::uint32_t& a_size, RE::TESImageSpaceModifier* a_imad, float a_default)
		{
			if (a_size == 0) {
				auto ichunk = a_file->GetTESChunk();
				const auto cchunk = reinterpret_cast<char*>(std::addressof(ichunk));
				if (cchunk[0] < '@') {
					cchunk[0] += 'a';
				}
				const std::string_view schunk{ cchunk, 4 };
				stl::report_and_fail(
					fmt::format(
						"IMAD with ID: [0x{:08X}] from \"{}\" with subrecord \"{}\" has an invalid key size of zero. "
						"This will result in memory corruption. "
						"Please open the form in xEdit and correct it or remove the mod from your load order."sv,
						a_imad->GetFormID(),
						a_file->GetFilename(),
						schunk));
			} else {
				return _original(a_file, a_size, a_imad->data.animatable, a_default);
			}
		}

		using func_t = RE::NiFloatInterpolator*(RE::TESFile*, std::uint32_t&, bool, float);
		static inline REL::Relocation<func_t> _original;
	};
}
