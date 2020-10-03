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
	void F4EE::SimpleInlinePatch(std::uintptr_t a_dst, std::size_t a_size, std::uintptr_t a_func)
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

		REL::safe_fill(a_dst, REL::NOP, a_size);
		Patch p{ a_func };
		p.ready();
		assert(p.getSize() <= a_size);
		REL::safe_write(
			a_dst,
			stl::span{ p.getCode<const std::byte*>(), p.getSize() });
	}

	void F4EE::SetMorphValues(std::uintptr_t a_base)
	{
		{
			constexpr std::size_t first = 0x001A1F8;
			constexpr std::size_t last = 0x001A23C;
			constexpr std::size_t size = last - first;
			const auto dst = a_base + first;
			SimpleInlinePatch(dst, size, reinterpret_cast<std::uintptr_t>(&AllocateMorphs));
		}

		{
			struct Patch :
				Xbyak::CodeGenerator
			{
				Patch(std::uintptr_t a_dst)
				{
					sub(rsi, r15);
					sar(rsi, 2);

					mov(rcx, r15);				// float*
					mov(rdx, rsi);				// size
					mov(r8, ptr[r14 + 0x2D8]);	// BSTArray<float>*

					mov(r9, a_dst);
					call(r9);
				}
			};

			constexpr std::size_t first = 0x001A253;
			constexpr std::size_t last = 0x001A2FF;
			constexpr std::size_t size = last - first;
			const auto dst = a_base + first;

			REL::safe_fill(dst, REL::NOP, size);
			Patch p{ reinterpret_cast<std::uintptr_t>(&CopyMorphs) };
			p.ready();
			assert(p.getSize() <= size);
			REL::safe_write(
				dst,
				stl::span{ p.getCode<const std::byte*>(), p.getSize() });
		}
	}
}
