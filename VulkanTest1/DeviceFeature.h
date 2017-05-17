#pragma once
#include "Enums.h"

enum class DeviceFeature
{
	SamplerAnisotropy
};

template<> __forceinline constexpr auto Enums::min<DeviceFeature>() { return Enums::value(DeviceFeature::SamplerAnisotropy); }
template<> __forceinline constexpr auto Enums::max<DeviceFeature>() { return Enums::value(DeviceFeature::SamplerAnisotropy); }

extern std::ostream& operator<<(std::ostream& lhs, DeviceFeature rhs);