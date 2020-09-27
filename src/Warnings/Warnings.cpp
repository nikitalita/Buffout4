#include "Warnings/Warnings.h"

#include "Warnings/ImageSpaceAdapterWarning.h"

namespace Warnings
{
	void PreInit()
	{
		if (*Settings::ImageSpaceAdapter) {
			ImageSpaceAdapterWarning::Install();
		}
	}
}
