#pragma once

struct _EXCEPTION_RECORD;

namespace Crash
{
	namespace Modules
	{
		class Module;
	}

	class Callstack
	{
	public:
		Callstack(const ::_EXCEPTION_RECORD& a_except) noexcept;

		Callstack(boost::stacktrace::stacktrace a_stacktrace) noexcept :
			_stacktrace{ std::move(a_stacktrace) },
			_frames{ stl::make_span(_stacktrace.begin(), _stacktrace.end()) }
		{}

		void print(
			std::shared_ptr<spdlog::logger> a_log,
			stl::span<const std::unique_ptr<Modules::Module>> a_modules) const noexcept;

	private:
		[[nodiscard]] static std::string get_size_string(std::size_t a_size) noexcept;

		[[nodiscard]] std::string get_format(std::size_t a_nameWidth) const noexcept;

		void print_probable_callstack(
			std::shared_ptr<spdlog::logger> a_log,
			stl::span<const std::unique_ptr<Modules::Module>> a_modules) const noexcept;

		void print_raw_callstack(std::shared_ptr<spdlog::logger> a_log) const noexcept;

		boost::stacktrace::stacktrace _stacktrace;
		stl::span<const boost::stacktrace::frame> _frames;
	};

	void Install();
}
