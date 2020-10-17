#include "Patches/MemoryManagerPatch.h"

#include "Crash/CrashHandler.h"
#include "Crash/Modules/ModuleHandler.h"

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

namespace Patches
{
	void MemoryManagerPatch::AutoScrapBuffer::CtorShort()
	{
		struct Patch :
			Xbyak::CodeGenerator
		{
			Patch()
			{
				mov(qword[rcx], 0);
				mov(rax, rcx);
				ret();
			}
		};

		REL::Relocation<std::uintptr_t> target{ REL::ID(1571567) };
		constexpr std::size_t size = 0x1C;
		REL::safe_fill(target.address(), REL::INT3, size);

		Patch p;
		p.ready();
		assert(p.getSize() <= size);
		REL::safe_write(
			target.address(),
			stl::span{ p.getCode<const std::byte*>(), p.getSize() });
	}

	void MemoryManagerPatch::AutoScrapBuffer::Dtor()
	{
		REL::Relocation<std::uintptr_t> base{ REL::ID(68625) };

		{
			struct Patch :
				Xbyak::CodeGenerator
			{
				Patch()
				{
					xor_(rax, rax);
					cmp(rbx, rax);
				}
			};

			const auto dst = base.address() + 0x9;
			constexpr std::size_t size = 0x1D;
			REL::safe_fill(dst, REL::NOP, size);

			Patch p;
			p.ready();
			assert(p.getSize() <= size);
			REL::safe_write(
				dst,
				stl::span{ p.getCode<const std::byte*>(), p.getSize() });
		}

		{
			const auto dst = base.address() + 0x26;
			REL::safe_write(dst, std::uint8_t{ 0x74 });  // jnz -> jz
		}
	}

	void MemoryManagerPatch::MemoryManager::DbgDeallocate(RE::MemoryManager* a_this, void* a_mem, bool a_alignmentRequired)
	{
		auto access = MemoryTraces::get().access();
		const auto it = access->find(a_mem);
		if (it != access->end() && it->second.first != a_alignmentRequired) {
			const auto log = spdlog::default_logger();
			log->set_pattern("%v"s);
			log->set_level(spdlog::level::trace);
			log->flush_on(spdlog::level::off);
			log->critical("");

			Crash::Callstack callstack{ std::move(it->second.second) };
			const auto modules = Crash::Modules::get_loaded_modules();
			callstack.print(
				log,
				stl::make_span(modules.begin(), modules.end()));

			log->flush();
			stl::report_and_fail("A bad deallocation has resulted in a crash. Please see Buffout4.log for more details."sv);
		} else {
			Deallocate(a_this, a_mem, a_alignmentRequired);
			access->erase(a_mem);
		}
	}
}
