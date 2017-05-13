#pragma once
#include <type_traits>

namespace Enums
{
	template<class T> __forceinline constexpr std::underlying_type_t<T> value(T val)
	{
		return std::underlying_type_t<T>(val);
	}

	template<class T> __forceinline constexpr auto min();
	template<class T> __forceinline constexpr auto max();
	template<class T> __forceinline constexpr std::underlying_type_t<T> count()
	{
		return max<T>() - min<T>() + 1;
	}

	template<class T> __forceinline constexpr T index_to_value(std::underlying_type_t<T> index)
	{
		return (T)(min<T>() + index);
	}
	template<class T> __forceinline constexpr std::underlying_type_t<T> value_to_index(T val)
	{
		return value(val) - min<T>();
	}

	template<class T> __forceinline constexpr bool validate(T val)
	{
		return value(val) >= min<T>() && value(val) <= max<T>();
	}
}