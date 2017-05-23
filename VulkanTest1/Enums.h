#pragma once
#include <type_traits>

namespace Enums
{
	template<class T> __forceinline constexpr std::underlying_type_t<T> value(T val)
	{
		return std::underlying_type_t<T>(val);
	}

	template<class T> __forceinline constexpr T add_flag(T val, T flag)
	{
		return T(Enums::value(val) | Enums::value(flag));
	}
	template<class T> __forceinline constexpr T remove_flag(T val, T flag)
	{
		return T(Enums::value(val) & ~Enums::value(flag));
	}

	template<class T> __forceinline constexpr bool has_flag(T val, T flag)
	{
		return (Enums::value(val) & Enums::value(flag)) == Enums::value(flag);
	}
	template<class T> __forceinline bool has_any_flag(T val, std::initializer_list<T> flags)
	{
		static_assert(std::is_enum_v<T>);

		T f = T();
		for (const T& current : flags)
			f = Enums::add_flag(f, current);

		return Enums::value(val) & Enums::value(f);
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

	template<class To, class From> inline constexpr To convert(From from);
	template<class To, class From> inline To convert(const std::optional<From>& from) { return convert<To, From>(from.value()); }
}