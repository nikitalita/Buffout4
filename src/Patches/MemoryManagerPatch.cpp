#include "Patches/MemoryManagerPatch.h"

#include "Crash/CrashHandler.h"
#include "Crash/Modules/ModuleHandler.h"

namespace Patches
{
	void MemoryManagerPatch::MemoryManager::DbgDeallocate(RE::MemoryManager*, void* a_mem, bool a_alignmentRequired)
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
			if (a_alignmentRequired) {
				_aligned_free(a_mem);
			} else {
				std::free(a_mem);
			}

			access->erase(a_mem);
		}
	}
}
