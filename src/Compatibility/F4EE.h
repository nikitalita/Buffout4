#pragma once

namespace Compatibility
{
	class F4EE
	{
	public:
		static void Install()
		{
			const auto validate = []() {
				// 1.6.20
				constexpr auto precalc = "D5467FEA7D6A722E0ED87B5FB857B5E0C9FDBE57B060ADB711013ED27565688D3D4514F99E4DF02109273ED69330E5272AD76822EB15717FEBC11AF409BA60F1"sv;

				const auto sha = Botan::HashFunction::create("SHA-512"s);
				boost::iostreams::mapped_file_source file("data/f4se/plugins/f4ee.dll");
				if (sha && file.is_open()) {
					const auto hash = sha->process(
						reinterpret_cast<const std::uint8_t*>(file.data()),
						file.size());
					std::string hashStr;
					hashStr.reserve(hash.size() * 2);
					for (const auto byte : hash) {
						hashStr += fmt::format(FMT_STRING("{:02X}"), byte);
					}

					if (precalc == hashStr) {
						return true;
					} else {
						logger::error(
							FMT_STRING(
								"{}: mismatch on sha512\n"
								"\texpected \"{}\"\n"
								"\tfound \"{}\""),
							typeid(F4EE).name(),
							precalc,
							hashStr);
					}
				}

				return false;
			};

			const auto handle = static_cast<const std::byte*>(WinAPI::GetModuleHandle(L"f4ee.dll"));
			if (handle != nullptr && validate()) {
				const auto base = reinterpret_cast<std::uintptr_t>(handle);
				SetMorphValues(base);
				logger::info("installed {}"sv, typeid(F4EE).name());
			} else {
				logger::warn("failed to install {}"sv, typeid(F4EE).name());
			}
		}

	private:
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

		static void SimpleInlinePatch(std::uintptr_t a_dst, std::size_t a_size, std::uintptr_t a_func);
		static void SetMorphValues(std::uintptr_t a_base);
	};
}
