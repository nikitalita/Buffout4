#pragma once

namespace Patches
{
	class MemoryManagerPatch
	{
	public:
		static void Install()
		{
			StubInit();
			ReplaceAllocRoutines();
			RE::MemoryManager::GetSingleton().RegisterMemoryManager();
			RE::BSThreadEvent::InitSDM();
			logger::info("installed {}"sv, typeid(MemoryManagerPatch).name());
		}

	private:
		static void* Allocate(RE::MemoryManager*, std::size_t a_size, std::uint32_t a_alignment, bool a_alignmentRequired)
		{
			if (a_alignmentRequired) {
				return _aligned_malloc(a_size, a_alignment);
			} else {
				return std::malloc(a_size);
			}
		}

		static void Deallocate(RE::MemoryManager*, void* a_mem, bool a_alignmentRequired)
		{
			if (a_alignmentRequired) {
				_aligned_free(a_mem);
			} else {
				std::free(a_mem);
			}
		}

		static void* Reallocate(RE::MemoryManager*, void* a_oldMem, std::size_t a_newSize, std::uint32_t a_alignment, bool a_alignmentRequired)
		{
			if (a_alignmentRequired) {
				return _aligned_realloc(a_oldMem, a_newSize, a_alignment);
			} else {
				return std::realloc(a_oldMem, a_newSize);
			}
		}

		static void ReplaceAllocRoutines()
		{
			using tuple_t = std::tuple<std::uint64_t, std::size_t, void*>;
			const std::array todo{
				tuple_t{ 652767, 0x24B, &Allocate },
				tuple_t{ 1582181, 0x115, &Deallocate },
				tuple_t{ 1502917, 0xA2, &Reallocate },
			};

			auto& trampoline = F4SE::GetTrampoline();
			for (const auto& [id, size, func] : todo) {
				REL::Relocation<std::uintptr_t> target{ REL::ID(id) };
				REL::safe_fill(target.address(), REL::INT3, size);
				trampoline.write_branch<6>(target.address(), func);
			}
		}

		static void StubInit()
		{
			REL::Relocation<std::uintptr_t> target{ REL::ID(597736) };
			REL::safe_fill(target.address(), REL::INT3, 0x9C);
			REL::safe_write(target.address(), REL::RET);
		}
	};
}
