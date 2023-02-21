#include "Patches/AchievementsPatch.h"

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

namespace Patches::AchievementsPatch
{
	namespace detail
	{
		struct Patch :
			Xbyak::CodeGenerator
		{
			Patch()
			{
				xor_(rax, rax);
				ret();
			}
		};
	}

	void Install()
	{
		REL::Relocation<std::uintptr_t> target{ REL::ID(1432894) };
		std::size_t size = REL::Module::IsF4() ? 0x73 : 0xDC;
		REL::safe_fill(target.address(), REL::INT3, size);

		detail::Patch p;
		p.ready();
		assert(p.getSize() < size);
		REL::safe_write(
			target.address(),
			std::span{ p.getCode<const std::byte*>(), p.getSize() });

		logger::info("installed Achievements patch"sv);
	}
}
