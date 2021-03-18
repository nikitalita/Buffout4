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

namespace Compatibility::F4EE::detail
{
	void SimpleInlinePatch(std::uintptr_t a_dst, std::size_t a_size, std::uintptr_t a_func)
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
			std::span{ p.getCode<const std::byte*>(), p.getSize() });
	}

	void SetMorphValues(std::uintptr_t a_base)
	{
		{
			constexpr std::size_t first = 0x001AB6E;
			constexpr std::size_t last = 0x001ABB9;
			constexpr std::size_t size = last - first;
			const auto dst = a_base + first;
			SimpleInlinePatch(dst, size, reinterpret_cast<std::uintptr_t>(&CreateMorphs));
		}

		{
			struct Patch :
				Xbyak::CodeGenerator
			{
				Patch(std::uintptr_t a_dst)
				{
					sub(rdi, rsi);
					sar(rdi, 2);

					mov(rcx, rsi);              // float*
					mov(rdx, rdi);              // size
					mov(r8, ptr[r15 + 0x2D8]);  // BSTArray<float>*

					mov(r9, a_dst);
					call(r9);
				}
			};

			constexpr std::size_t first = 0x001ABC9;
			constexpr std::size_t last = 0x001AC78;
			constexpr std::size_t size = last - first;
			const auto dst = a_base + first;

			REL::safe_fill(dst, REL::NOP, size);
			Patch p{ reinterpret_cast<std::uintptr_t>(&CopyMorphs) };
			p.ready();
			assert(p.getSize() <= size);
			REL::safe_write(
				dst,
				std::span{ p.getCode<const std::byte*>(), p.getSize() });
		}
	}
}
