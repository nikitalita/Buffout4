#pragma once

namespace Fixes
{
	class UtilityShaderFix
	{
	public:
		static void Install()
		{
			REL::Relocation<std::uintptr_t> base{ REL::ID(768994) };
			CreateShaders();
			PatchPixelShader(base.address());
			PatchVertexShader(base.address());
			logger::info("installed {}"sv, typeid(UtilityShaderFix).name());
		}

	private:
		static void CreateShaders()
		{
			REL::Relocation<void()> func{ REL::ID(527640) };
			func();
		}

		static void PatchPixelShader(std::uintptr_t a_base);
		static void PatchVertexShader(std::uintptr_t a_base);
	};
}
