#include "PCH.h"

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

#include <Windows.h>

#include <dbghelp.h>

#include <xbyak/xbyak.h>

namespace WinAPI
{
	bool(IsDebuggerPresent)() noexcept
	{
		return static_cast<bool>(
			::IsDebuggerPresent());
	}

	void(OutputDebugStringA)(
		const char* a_outputString) noexcept
	{
		::OutputDebugStringA(
			static_cast<::LPCSTR>(a_outputString));
	}

	std::uint32_t(UnDecorateSymbolName)(
		const char* a_name,
		char* a_outputString,
		std::uint32_t a_maxStringLength,
		std::uint32_t a_flags) noexcept
	{
		return static_cast<std::uint32_t>(
			::UnDecorateSymbolName(
				static_cast<::PCSTR>(a_name),
				static_cast<::PSTR>(a_outputString),
				static_cast<::DWORD>(a_maxStringLength),
				static_cast<::DWORD>(a_flags)));
	}
}

namespace stl
{
	namespace detail
	{
		struct asm_call_t
		{};

		struct asm_jump_t
		{};

		struct asm_patch :
			Xbyak::CodeGenerator
		{
			asm_patch(
				std::variant<asm_call_t, asm_jump_t> a_type,
				std::uintptr_t a_dst)
			{
				Xbyak::Label lbl;

				if (std::holds_alternative<asm_call_t>(a_type)) {
					call(ptr[rip + lbl]);
				} else if (std::holds_alternative<asm_jump_t>(a_type)) {
					jmp(ptr[rip + lbl]);
				} else {
					stl::report_and_fail("fatal failure"sv);
				}

				L(lbl);
				dq(a_dst);
			}
		};

		void asm_write(
			std::variant<asm_call_t, asm_jump_t> a_type,
			std::uintptr_t a_from,
			[[maybe_unused]] std::size_t a_size,
			std::uintptr_t a_to)
		{
			detail::asm_patch p{ a_type, a_to };
			p.ready();
			assert(p.getSize() <= a_size);
			REL::safe_write(
				a_from,
				stl::span{ p.getCode<const std::byte*>(), p.getSize() });
		}
	}

	void asm_call(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to) { detail::asm_write(detail::asm_call_t{}, a_from, a_size, a_to); }
	void asm_jump(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to) { detail::asm_write(detail::asm_jump_t{}, a_from, a_size, a_to); }
}
