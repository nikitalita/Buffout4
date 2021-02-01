#pragma once

namespace Patches::SmallBlockAllocatorPatch
{
	namespace detail
	{
		void InstallAllocations();
		void InstallDeallocations();

		inline void* Allocate(std::size_t a_size)
		{
			return a_size > 0 ?
                       scalable_aligned_malloc(a_size, 0x10) :
                       nullptr;
		}

		inline void Deallocate(void* a_ptr)
		{
			scalable_aligned_free(a_ptr);
		}
	}

	inline void Install()
	{
		detail::InstallAllocations();
		detail::InstallDeallocations();

		REL::Relocation<std::uintptr_t> target{ REL::ID(329149), 0x48 };
		REL::safe_fill(target.address(), REL::NOP, 0x5);

		logger::info("installed SmallBlockAllocator patch"sv);
	}
}
