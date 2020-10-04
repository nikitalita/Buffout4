#include "Fixes/UtilityShaderFix.h"

#define WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

#include <xbyak/xbyak.h>

namespace Fixes
{
	void WritePatch(std::uintptr_t a_base, std::size_t a_first, std::size_t a_last, const Xbyak::CodeGenerator& a_code)
	{
		const std::size_t size = a_last - a_first;
		const auto dst = a_base + a_first;
		REL::safe_fill(dst, REL::NOP, size);

		auto& trampoline = F4SE::GetTrampoline();
		assert(size >= 6);
		trampoline.write_call<6>(
			dst,
			trampoline.allocate(a_code));
	}

	void UtilityShaderFix::PatchPixelShader(std::uintptr_t a_base)
	{
		struct Patch :
			Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_data)
			{
				mov(rax, a_data);
				ret();
			}
		};

		REL::Relocation<RE::BSGraphics::PixelShader**> shader{ REL::ID(286285) };

		Patch p{ reinterpret_cast<std::uintptr_t>(*shader) };
		p.ready();
		WritePatch(a_base, 0x1A4, 0x1AB, p);
	}

	void UtilityShaderFix::PatchVertexShader(std::uintptr_t a_base)
	{
		struct Patch :
			Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_data)
			{
				mov(r13, a_data);
				ret();
			}
		};

		REL::Relocation<RE::BSGraphics::VertexShader**> shader{ REL::ID(67091) };

		Patch p{ reinterpret_cast<std::uintptr_t>(*shader) };
		p.ready();
		WritePatch(a_base, 0x150, 0x157, p);
	}
}
