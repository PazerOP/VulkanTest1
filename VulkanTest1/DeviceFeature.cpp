#include "stdafx.h"
#include "DeviceFeature.h"

std::ostream& operator<<(std::ostream& lhs, DeviceFeature rhs)
{
	switch (rhs)
	{
	case DeviceFeature::SamplerAnisotropy:	return lhs << "DeviceFeature::SamplerAnisotropy";
	}

	assert(!false);
	return lhs << Enums::value(rhs);
}
