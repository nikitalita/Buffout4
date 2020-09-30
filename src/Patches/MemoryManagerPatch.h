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
		class MemoryTraces
		{
		public:
			using lock_type = std::mutex;
			using value_type = robin_hood::unordered_flat_map<void*, std::pair<bool, boost::stacktrace::stacktrace>>;

			class Accessor
			{
			public:
				Accessor() = delete;
				Accessor(const Accessor&) = delete;
				Accessor(Accessor&&) = delete;

				~Accessor() = default;

				Accessor& operator=(const Accessor&) = delete;
				Accessor& operator=(Accessor&&) = delete;

				[[nodiscard]] value_type& operator*() noexcept { return get(); }
				[[nodiscard]] const value_type& operator*() const noexcept { return get(); }

				[[nodiscard]] value_type* operator->() noexcept { return std::addressof(get()); }
				[[nodiscard]] const value_type* operator->() const noexcept { return std::addressof(get()); }

				[[nodiscard]] value_type& get() noexcept { return _proxy; }
				[[nodiscard]] const value_type& get() const noexcept { return _proxy; }

			protected:
				friend class MemoryTraces;

				Accessor(lock_type& a_lock, value_type& a_proxy) :
					_locker(a_lock),
					_proxy(a_proxy)
				{}

			private:
				std::scoped_lock<lock_type> _locker;
				value_type& _proxy;
			};

			[[nodiscard]] static MemoryTraces& get() noexcept
			{
				static MemoryTraces singleton;
				return singleton;
			}

			[[nodiscard]] Accessor access() { return { _lock, _traces }; }

		private:
			MemoryTraces() noexcept = default;
			MemoryTraces(const MemoryTraces&) = delete;
			MemoryTraces(MemoryTraces&&) = delete;

			~MemoryTraces() noexcept = default;

			MemoryTraces& operator=(const MemoryTraces&) = delete;
			MemoryTraces& operator=(MemoryTraces&&) = delete;

			lock_type _lock;
			value_type _traces;
		};

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
				return a_alignmentRequired ?
							 scalable_aligned_malloc(a_size, a_alignment) :
							 scalable_malloc(a_size);
			}

			static void* DbgAllocate(RE::MemoryManager* a_this, std::size_t a_size, std::uint32_t a_alignment, bool a_alignmentRequired)
			{
				const auto result = Allocate(a_this, a_size, a_alignment, a_alignmentRequired);
				if (result && a_size == sizeof(RE::NiNode)) {
					auto access = MemoryTraces::get().access();
					access->emplace(
						result,
						std::make_pair(a_alignmentRequired, boost::stacktrace::stacktrace{}));
				}
				return result;
			}

			static void Deallocate(RE::MemoryManager*, void* a_mem, bool a_alignmentRequired)
			{
				a_alignmentRequired ?
					  scalable_aligned_free(a_mem) :
					  scalable_free(a_mem);
			}

			static void DbgDeallocate(RE::MemoryManager* a_this, void* a_mem, bool a_alignmentRequired);

			static void* Reallocate(RE::MemoryManager*, void* a_oldMem, std::size_t a_newSize, std::uint32_t a_alignment, bool a_alignmentRequired)
			{
				return a_alignmentRequired ?
							 scalable_aligned_realloc(a_oldMem, a_newSize, a_alignment) :
							 scalable_realloc(a_oldMem, a_newSize);
			}

			static void* DbgReallocate(RE::MemoryManager* a_this, void* a_oldMem, std::size_t a_newSize, std::uint32_t a_alignment, bool a_alignmentRequired)
			{
				auto access = MemoryTraces::get().access();
				const auto result = Reallocate(a_this, a_oldMem, a_newSize, a_alignment, a_alignmentRequired);
				if (const auto it = access->find(a_oldMem); it != access->end()) {
					access->erase(it);
					access->emplace(
						result,
						std::make_pair(a_alignmentRequired, boost::stacktrace::stacktrace{}));
				}
				return result;
			}

			static void ReplaceAllocRoutines()
			{
				using tuple_t = std::tuple<std::uint64_t, std::size_t, void*>;
				const std::array todo{
					tuple_t{ 652767, 0x24B, *Settings::MemoryManagerDebug ? &DbgAllocate : &Allocate },
					tuple_t{ 1582181, 0x115, *Settings::MemoryManagerDebug ? &DbgDeallocate : &Deallocate },
					tuple_t{ 1502917, 0xA2, *Settings::MemoryManagerDebug ? &DbgReallocate : &Reallocate },
				};

				for (const auto& [id, size, func] : todo) {
					REL::Relocation<std::uintptr_t> target{ REL::ID(id) };
					REL::safe_fill(target.address(), REL::INT3, size);
					stl::asm_jump(target.address(), size, reinterpret_cast<std::uintptr_t>(func));
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
				return scalable_aligned_malloc(a_size, a_alignment);
			}

			static RE::ScrapHeap* Ctor(RE::ScrapHeap* a_this)
			{
				std::memset(a_this, 0, sizeof(RE::ScrapHeap));
				stl::emplace_vtable(a_this);
				return a_this;
			}

			static void Deallocate(RE::ScrapHeap*, void* a_mem)
			{
				scalable_aligned_free(a_mem);
			}

			static void WriteHooks()
			{
				using tuple_t = std::tuple<std::uint64_t, std::size_t, void*>;
				const std::array todo{
					tuple_t{ 1085394, 0x5F6, &Allocate },
					tuple_t{ 923307, 0x144, &Deallocate },
					tuple_t{ 48809, 0x12B, &Ctor },
				};

				for (const auto& [id, size, func] : todo) {
					REL::Relocation<std::uintptr_t> target{ REL::ID(id) };
					REL::safe_fill(target.address(), REL::INT3, size);
					stl::asm_jump(target.address(), size, reinterpret_cast<std::uintptr_t>(func));
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
