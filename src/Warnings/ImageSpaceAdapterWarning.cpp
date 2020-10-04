#include "Warnings/ImageSpaceAdapterWarning.h"

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

namespace Warnings
{
	namespace
	{
		struct Patch :
			Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_dst)
			{
				Xbyak::Label dst;

				mov(r8, r14);
				jmp(ptr[rip + dst]);

				L(dst);
				dq(a_dst);
			}
		};
	}

	void ImageSpaceAdapterWarning::Install()
	{
		auto& trampoline = F4SE::GetTrampoline();
		REL::Relocation<std::uintptr_t> target{ REL::ID(231868), 0x57F };
		Patch p{ reinterpret_cast<std::uintptr_t>(&LoadChunk) };
		p.ready();
		_original = trampoline.write_call<5>(
			target.address(),
			trampoline.allocate(p));
		logger::info("installed {}"sv, typeid(ImageSpaceAdapterWarning).name());
	}
}
