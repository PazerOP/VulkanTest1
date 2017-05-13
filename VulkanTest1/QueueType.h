#pragma once
#include "Enums.h"

enum class QueueType
{
	Graphics,
	Presentation,
	Transfer,
};

template<> __forceinline constexpr auto Enums::min<QueueType>() { return Enums::value(QueueType::Graphics); }
template<> __forceinline constexpr auto Enums::max<QueueType>() { return Enums::value(QueueType::Transfer); }