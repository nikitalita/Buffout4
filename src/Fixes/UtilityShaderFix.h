#pragma once

namespace Fixes::UtilityShaderFix
{
	namespace detail
	{
		inline void CreateShaders()
		{
#ifndef FALLOUTVR
			REL::Relocation<void()> func{ REL::ID(527640) };
#else
			REL::Relocation<void()> func{ REL::Offset(0x250fa20) };
#endif
			func();
		}

		void PatchPixelShader(std::uintptr_t a_base);
		void PatchVertexShader(std::uintptr_t a_base);
	}

	inline void Install()
	{
		REL::Relocation<std::uintptr_t> base{ REL::ID(768994) };
		detail::CreateShaders();
		detail::PatchPixelShader(base.address());
		detail::PatchVertexShader(base.address());
		logger::debug("installed UtilityShader fix"sv);
	}
}
