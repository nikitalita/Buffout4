#pragma once

#include "F4SE/F4SE.h"
#include "RE/Fallout.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <execution>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <mutex>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <variant>
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4706)  // assignment within conditional expression
#include <boost/algorithm/searching/knuth_morris_pratt.hpp>
#include <boost/container/map.hpp>
#include <boost/nowide/convert.hpp>
#include <boost/stacktrace.hpp>
#include <fmt/chrono.h>
#include <frozen/map.h>
#include <infoware/cpu.hpp>
#include <infoware/gpu.hpp>
#include <infoware/system.hpp>
#include <robin_hood.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <tbb/scalable_allocator.h>

#include "AutoTOML.hpp"
#pragma warning(pop)

namespace WinAPI
{
	using namespace F4SE::WinAPI;

	inline constexpr auto(DLL_PROCESS_DETACH){ static_cast<unsigned long>(0) };
	inline constexpr auto(DLL_PROCESS_ATTACH){ static_cast<unsigned long>(1) };
	inline constexpr auto(DLL_THREAD_ATTACH){ static_cast<unsigned long>(2) };
	inline constexpr auto(DLL_THREAD_DETACH){ static_cast<unsigned long>(3) };

	inline constexpr auto(EXCEPTION_EXECUTE_HANDLER){ static_cast<int>(1) };

	inline constexpr auto(FALSE){ static_cast<int>(0) };
	inline constexpr auto(TRUE){ static_cast<int>(1) };

	inline constexpr auto(UNDNAME_NO_MS_KEYWORDS){ static_cast<std::uint32_t>(0x0002) };
	inline constexpr auto(UNDNAME_NO_FUNCTION_RETURNS){ static_cast<std::uint32_t>(0x0004) };
	inline constexpr auto(UNDNAME_NO_ALLOCATION_MODEL){ static_cast<std::uint32_t>(0x0008) };
	inline constexpr auto(UNDNAME_NO_ALLOCATION_LANGUAGE){ static_cast<std::uint32_t>(0x0010) };
	inline constexpr auto(UNDNAME_NO_THISTYPE){ static_cast<std::uint32_t>(0x0060) };
	inline constexpr auto(UNDNAME_NO_ACCESS_SPECIFIERS){ static_cast<std::uint32_t>(0x0080) };
	inline constexpr auto(UNDNAME_NO_THROW_SIGNATURES){ static_cast<std::uint32_t>(0x0100) };
	inline constexpr auto(UNDNAME_NO_RETURN_UDT_MODEL){ static_cast<std::uint32_t>(0x0400) };
	inline constexpr auto(UNDNAME_NAME_ONLY){ static_cast<std::uint32_t>(0x1000) };
	inline constexpr auto(UNDNAME_NO_ARGUMENTS){ static_cast<std::uint32_t>(0x2000) };

	[[nodiscard]] bool(IsDebuggerPresent)() noexcept;

	void(OutputDebugStringA)(
		const char* a_outputString) noexcept;

	[[nodiscard]] std::uint32_t(UnDecorateSymbolName)(
		const char* a_name,
		char* a_outputString,
		std::uint32_t a_maxStringLength,
		std::uint32_t a_flags) noexcept;
}

#ifndef NDEBUG
#include <spdlog/sinks/base_sink.h>

namespace logger
{
	template <class Mutex>
	class msvc_sink :
		public spdlog::sinks::base_sink<Mutex>
	{
	private:
		using super = spdlog::sinks::base_sink<Mutex>;

	public:
		explicit msvc_sink() {}

	protected:
		void sink_it_(const spdlog::details::log_msg& a_msg) override
		{
			spdlog::memory_buf_t formatted;
			super::formatter_->format(a_msg, formatted);
			WinAPI::OutputDebugStringA(fmt::to_string(formatted).c_str());
		}

		void flush_() override {}
	};

	using msvc_sink_mt = msvc_sink<std::mutex>;
	using msvc_sink_st = msvc_sink<spdlog::details::null_mutex>;

	using windebug_sink_mt = msvc_sink_mt;
	using windebug_sink_st = msvc_sink_st;
}
#endif

#define DLLEXPORT __declspec(dllexport)

namespace logger
{
	using namespace F4SE::log;
}

namespace stl
{
	using F4SE::stl::adjust_pointer;
	using F4SE::stl::emplace_vtable;
	using F4SE::stl::report_and_fail;
	using F4SE::stl::scope_exit;
	using F4SE::stl::to_underlying;

	void asm_jump(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to);
}

namespace util
{
	[[nodiscard]] inline std::string module_name()
	{
		const auto filename = REL::Module::get().filename();
		return boost::nowide::narrow(filename.data(), filename.length());
	}
}

using namespace std::literals;

#include "Version.h"

#include "Settings.h"
