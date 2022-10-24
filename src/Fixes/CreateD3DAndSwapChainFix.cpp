#include "Fixes/CreateD3DAndSwapChainFix.h"

#include <Windows.h>

#include <dxgi.h>

namespace CreateD3DAndSwapChainFix
{
	namespace detail
	{
		::HRESULT GetDisplayModeList(
			::IDXGIOutput& a_this,
			::DXGI_FORMAT a_enumFormat,
			::UINT a_flags,
			::UINT* a_numModes,
			::DXGI_MODE_DESC* a_desc)
		{
			const auto result = a_this.GetDisplayModeList(a_enumFormat, a_flags, a_numModes, a_desc);

			const auto modes = std::span(a_desc, *a_numModes);
			const auto end = std::stable_partition(
				modes.begin(),
				modes.end(),
				[](const ::DXGI_MODE_DESC& a_desc) {
					return a_desc.RefreshRate.Denominator != 0;
				});
			*a_numModes = static_cast<::UINT>(end - modes.begin());

			return result;
		}
	}

	void Install()
	{
#ifndef FALLOUTVR
		const auto target = REL::ID(224250).address() + 0x114;
#else
		const auto target = REL::Offset(0x1d9bc60).address() + 0x11F;
#endif
		auto& trampoline = F4SE::GetTrampoline();
		trampoline.write_call<5>(target, detail::GetDisplayModeList);

		logger::debug("installed CreateD3DAndSwapChain fix"sv);
	}
}
