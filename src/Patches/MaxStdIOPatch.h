#pragma once

namespace Patches
{
	class MaxStdIOPatch
	{
	public:
		static void Install()
		{
			const auto handle = WinAPI::GetModuleHandle(L"api-ms-win-crt-stdio-l1-1-0.dll");
			const auto proc =
				handle ?
					  reinterpret_cast<decltype(&_setmaxstdio)>(WinAPI::GetProcAddress(handle, "_setmaxstdio")) :
					  nullptr;
			if (proc != nullptr) {
				const auto result = proc(static_cast<int>(*Settings::MaxStdIO));
				logger::info("set max stdio to {}"sv, result);
			} else {
				logger::error("failed to install {}"sv, typeid(MaxStdIOPatch).name());
			}
		}
	};
}
