#include "Compatibility/F4EE.h"

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

namespace Compatibility
{
	struct Patch :
		Xbyak::CodeGenerator
	{
		Patch(std::uintptr_t a_dst)
		{
			mov(rcx, a_dst);
			call(rcx);
		}
	};

	void F4EE::UpdateOverlays(std::uintptr_t a_base)
	{
		constexpr std::uintptr_t offset = 0x0043416;
		constexpr std::size_t size = 0x004343E - offset;
		const auto dst = a_base + offset;
		REL::safe_fill(dst, REL::NOP, size);
		Patch p{ reinterpret_cast<std::uintptr_t>(&Allocate) };
		p.ready();
		assert(p.getSize() <= size);
		REL::safe_write(
			dst,
			stl::span{ p.getCode<const std::byte*>(), p.getSize() });
	}
}
