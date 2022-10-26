#pragma once

namespace Fixes::GreyMoviesFix
{
	namespace detail
	{
		inline void SetBackgroundAlpha(RE::Scaleform::GFx::Movie& a_self, float)
		{
			RE::Scaleform::GFx::Value alpha;
			if (!a_self.GetVariable(std::addressof(alpha), "BackgroundAlpha")) {
				alpha = 0.0;
			}

			a_self.SetBackgroundAlpha(static_cast<float>(alpha.GetNumber()));
		}
	}

	inline void Install()
	{
		const REL::Relocation<std::uintptr_t> target{ REL::ID(1526234), 0x216 };
		auto& trampoline = F4SE::GetTrampoline();
		trampoline.write_call<6>(target.address(), detail::SetBackgroundAlpha);
		logger::info("installed GreyMovies fix"sv);
	}
}
