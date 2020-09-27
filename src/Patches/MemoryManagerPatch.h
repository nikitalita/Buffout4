#pragma once

namespace Patches
{
	class MemoryManagerPatch
	{
	public:
		static void Install()
		{
			MemoryManager::Install();
			ScrapHeap::Install();

			logger::info("installed {}"sv, typeid(MemoryManagerPatch).name());
		}

	private:
		class MemoryManager
		{
		public:
			static void Install()
			{
				StubInit();
				ReplaceAllocRoutines();
				RE::MemoryManager::GetSingleton().RegisterMemoryManager();
				RE::BSThreadEvent::InitSDM();
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
					trampoline.write_branch<6>(target.address(), func);	 // TODO: optimize out trampoline usage
				}
			}

			static void StubInit()
			{
				REL::Relocation<std::uintptr_t> target{ REL::ID(597736) };
				REL::safe_fill(target.address(), REL::INT3, 0x9C);
				REL::safe_write(target.address(), REL::RET);
			}
		};

		class ScrapHeap
		{
		public:
			static void Install()
			{
				WriteStubs();
				WriteHooks();
			}

		private:
			static void* Allocate(RE::ScrapHeap*, std::size_t a_size, std::size_t a_alignment)
			{
				return _aligned_malloc(a_size, a_alignment);
			}

			static RE::ScrapHeap* Ctor(RE::ScrapHeap* a_this)
			{
				std::memset(a_this, 0, sizeof(RE::ScrapHeap));
				reinterpret_cast<std::uintptr_t*>(a_this)[0] = RE::ScrapHeap::VTABLE[0].address();
				return a_this;
			}

			static void Deallocate(RE::ScrapHeap*, void* a_mem)
			{
				_aligned_free(a_mem);
			}

			static void WriteHooks()
			{
				using tuple_t = std::tuple<std::uint64_t, std::size_t, void*>;
				const std::array todo{
					tuple_t{ 1085394, 0x5F6, Allocate },
					tuple_t{ 923307, 0x144, Deallocate },
					tuple_t{ 48809, 0x12B, Ctor },
				};

				auto& trampoline = F4SE::GetTrampoline();
				for (const auto& [id, size, func] : todo) {
					REL::Relocation<std::uintptr_t> target{ REL::ID(id) };
					REL::safe_fill(target.address(), REL::INT3, size);
					trampoline.write_branch<6>(target.address(), func);	 // TODO: optimize out trampoline usage
				}
			}

			static void WriteStubs()
			{
				using tuple_t = std::tuple<std::uint64_t, std::size_t>;
				const std::array todo{
					tuple_t{ 550677, 0xC3 },  // Clean
					tuple_t{ 111657, 0x8 },	  // ClearKeepPages
					tuple_t{ 975239, 0xF6 },  // InsertFreeBlock
					tuple_t{ 84225, 0x183 },  // RemoveFreeBlock
					tuple_t{ 1255203, 0x4 },  // SetKeepPages
					tuple_t{ 912706, 0x32 },  // dtor
				};

				for (const auto& [id, size] : todo) {
					REL::Relocation<std::uintptr_t> target{ REL::ID(id) };
					REL::safe_fill(target.address(), REL::INT3, size);
					REL::safe_write(target.address(), REL::RET);
				}
			}
		};
	};
}
