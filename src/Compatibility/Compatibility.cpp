#include "Compatibility/Compatibility.h"

#include "Compatibility/ClassicHolsteredWeapons.h"
#include "Compatibility/F4EE.h"

namespace Compatibility
{
	void Install()
	{
		if (*Settings::ClassicHolsteredWeapons) {
			ClassicHolsteredWeapons::Install();
		}

		if (*Settings::F4EE) {
			F4EE::Install();
		}
	}
}
