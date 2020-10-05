#pragma once

namespace Patches
{
	class SmallBlockAllocatorPatch
	{
	public:
		static void Install()
		{
			InstallAllocations();
			InstallDeallocations();

			REL::Relocation<std::uintptr_t> target{ REL::ID(329149), 0x48 };
			REL::safe_fill(target.address(), REL::NOP, 0x5);

			logger::info("installed {}"sv, typeid(SmallBlockAllocatorPatch).name());
		}

	private:
		static void InstallAllocations();
		static void InstallDeallocations();

		static void* Allocate(std::size_t a_size)
		{
			return a_size > 0 ?
						 scalable_aligned_malloc(a_size, 0x10) :
						 nullptr;
		}

		static void Deallocate(void* a_ptr)
		{
			scalable_aligned_free(a_ptr);
		}
	};
}
