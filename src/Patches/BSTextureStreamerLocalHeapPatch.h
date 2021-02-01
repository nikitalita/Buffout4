#pragma once

namespace Patches::BSTextureStreamerLocalHeapPatch
{
	namespace detail
	{
		inline void* Allocate(RE::BSTextureStreamer::LocalHeap*, std::uint32_t a_size)
		{
			return a_size > 0 ?
                       scalable_aligned_malloc(a_size, 0x10) :
                       nullptr;
		}

		inline RE::BSTextureStreamer::LocalHeap* Ctor(RE::BSTextureStreamer::LocalHeap* a_this)
		{
			std::memset(a_this, 0, sizeof(RE::BSTextureStreamer::LocalHeap));
			return a_this;
		}

		inline void Deallocate(RE::BSTextureStreamer::LocalHeap*, void* a_ptr)
		{
			scalable_aligned_free(a_ptr);
		}

		inline void WriteHooks()
		{
			using tuple_t = std::tuple<std::uint64_t, std::size_t, void*>;
			const std::array todo{
				tuple_t{ 790612, 0x1B5, &Allocate },
				tuple_t{ 1493823, 0x74, &Ctor },
				tuple_t{ 1576443, 0x149, &Deallocate },
			};

			for (const auto& [id, size, func] : todo) {
				REL::Relocation<std::uintptr_t> target{ REL::ID(id) };
				REL::safe_fill(target.address(), REL::INT3, size);
				stl::asm_jump(target.address(), size, reinterpret_cast<std::uintptr_t>(func));
			}
		}

		inline void WriteStubs()
		{
			using tuple_t = std::tuple<std::uint64_t, std::size_t>;
			const std::array todo{
				tuple_t{ 1100993, 0x7A },  // InsertFreeBlock
				tuple_t{ 318310, 0x11F },  // RemoveFreeBlock
			};

			for (const auto& [id, size] : todo) {
				REL::Relocation<std::uintptr_t> target{ REL::ID(id) };
				REL::safe_fill(target.address(), REL::INT3, size);
				REL::safe_write(target.address(), REL::RET);
			}
		}
	}

	inline void Install()
	{
		detail::WriteStubs();
		detail::WriteHooks();
		logger::info("installed BSTextureStreamerLocalHeap patch"sv);
	}
}
