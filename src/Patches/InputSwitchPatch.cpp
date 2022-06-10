#include "Patches/InputSwitchPatch.h"

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

namespace Patches::InputSwitchPatch::detail
{
	namespace
	{
		static void PipboyMenuPreDtor()
		{
			if (const auto controls = RE::ControlMap::GetSingleton(); controls) {
				using RE::UserEvents::INPUT_CONTEXT_ID::kLThumbCursor;
				while (controls->PopInputContext(kLThumbCursor)) {}
			}
		}

		struct DtorPatch :
			Xbyak::CodeGenerator
		{
			DtorPatch(std::uintptr_t a_ret)
			{
				push(rcx);
				sub(rsp, 0x8);   // alignment
				sub(rsp, 0x20);  // function call
				mov(rax, reinterpret_cast<std::uintptr_t>(PipboyMenuPreDtor));
				call(rax);
				add(rsp, 0x20);
				add(rsp, 0x8);
				pop(rcx);

				// restore
				mov(ptr[rsp + 0x8], rbx);
				mov(ptr[rsp + 0x10], rbp);

				mov(rax, a_ret);
				jmp(rax);
			}
		};
	}

	void InstallPipboyMenuStatePatches()
	{
		const auto patch = []<class T>(std::in_place_type_t<T>, REL::ID a_func, std::size_t a_size) {
			assert(a_size >= 6);

			const auto target = a_func.address();
			REL::safe_fill(target, REL::NOP, a_size);

			T p{ target + a_size };
			p.ready();

			auto& trampoline = F4SE::GetTrampoline();
			trampoline.write_branch<6>(
				target,
				trampoline.allocate(p));
		};

		patch(std::in_place_type<DtorPatch>, REL::ID(405150), 0xA);
	}
}
