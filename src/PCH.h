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
#include <queue>
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
#include <boost/algorithm/searching/knuth_morris_pratt.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/nowide/convert.hpp>
#include <boost/stacktrace.hpp>
#include <fmt/chrono.h>
#include <frozen/map.h>
#include <infoware/cpu.hpp>
#include <infoware/gpu.hpp>
#include <infoware/system.hpp>
#include <robin_hood.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <tbb/scalable_allocator.h>

#include "AutoTOML.hpp"
#pragma warning(pop)

namespace WinAPI
{
	using namespace F4SE::WinAPI;

	inline constexpr auto(EXCEPTION_EXECUTE_HANDLER){ static_cast<int>(1) };

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

	[[nodiscard]] std::uint32_t(UnDecorateSymbolName)(
		const char* a_name,
		char* a_outputString,
		std::uint32_t a_maxStringLength,
		std::uint32_t a_flags) noexcept;
}

#define DLLEXPORT __declspec(dllexport)

namespace logger = F4SE::log;

namespace stl
{
	using F4SE::stl::adjust_pointer;
	using F4SE::stl::emplace_vtable;
	using F4SE::stl::report_and_fail;
	using F4SE::stl::scope_exit;
	using F4SE::stl::to_underlying;

	void asm_jump(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to);

	template <std::size_t N, class T>
	void write_thunk_call(std::uintptr_t a_src)
	{
		auto& trampoline = F4SE::GetTrampoline();
		T::func = trampoline.write_call<N>(a_src, T::thunk);
	}
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
