#pragma once

#include <algorithm>
#include <functional>
#include <type_traits>
#include <utility>
#include <variant>

#ifdef _DEBUG
#include <cassert>
#define AssertAR(before, statement, after)	assert(before statement after)
#else
#define AssertAR(before, statement, after)	(statement)
#endif

#if _MSC_VER == 1910 || _MSC_VER == 1911
namespace std
{
	namespace experimental
	{
		namespace filesystem
		{
			inline namespace v1
			{
				class path;
			}
		}
	}
	namespace filesystem = ::std::experimental::filesystem;
}
#else
namespace std
{
	namespace filesystem
	{
		class path;
	}
}
#endif

#undef min
#undef max

#define SV_MACRO(x)	std::string_view(x, std::size(x))

template<class TOut, class TIn>
inline TOut overflow_check(const TIn& in)
{
	assert(in >= std::numeric_limits<TOut>::min());
	assert(in <= std::numeric_limits<TOut>::max());
	return (TOut)in;
}

class programmer_error : public std::logic_error
{
public:
	programmer_error(const char* msg) : std::logic_error(msg) { }
	programmer_error(const std::string& msg) : std::logic_error(msg) { }
};

class not_implemented_error : public programmer_error
{
public:
	not_implemented_error() : programmer_error("function not implemented!") { }
	not_implemented_error(const char* msg) : programmer_error(msg) { }
	not_implemented_error(const std::string& msg) : programmer_error(msg) { }
};

// std::make_array is still missing from from the stl??
template<class T, class... Args>
constexpr std::array<T, sizeof...(Args)> make_array(Args&&... t)
{
	return { std::forward<Args>(t)... };
}

// Somehow didn't get added until C++17...
template< class T, class U >
std::shared_ptr<T> reinterpret_pointer_cast(const std::shared_ptr<U>& r) noexcept
{
	auto p = reinterpret_cast<typename std::shared_ptr<T>::element_type*>(r.get());
	return std::shared_ptr<T>(r, p);
}

// Remaps x from [0, 1] to [a, b]
__forceinline constexpr float Lerp(float a, float b, float x)
{
	return a + (b - a) * x;
}
// Remaps t from [x, y] to [a, b]
__forceinline constexpr float Remap(float a, float b, float x, float y, float t)
{
	return Lerp(a, b, (t - x) / (y - x));
}
// Remaps t from [x, y] to [a, b], clamping output between [a, b]
__forceinline constexpr float RemapClamped(float a, float b, float x, float y, float t)
{
	return (a < b) ? std::clamp(Remap(a, b, x, y, t), a, b) : std::clamp(Remap(a, b, x, y, t), b, a);
}

// Easier to read than const_cast<const SomeClass*>(this)
//template<class T> __forceinline constexpr const T* const_this(const T* thisPtr)
//{
//	return thisPtr;
//}

template<class T> __forceinline std::weak_ptr<T> weaken(const std::shared_ptr<T>& strong)
{
	return strong;
}
template<class T> __forceinline const std::weak_ptr<T>& weaken(const std::weak_ptr<T>& weak)
{
	return weak;
}

// Gets the relative path and removes the file extension.
extern std::string name_from_path(const std::filesystem::path& basePath, const std::filesystem::path& fullPath, bool removeExt = true);

namespace detail
{
	template<class T, class VariantType, size_t I>
	class variant_type_index_impl
	{
		using DecayedVariantType = std::decay_t<VariantType>;

	public:
		static constexpr size_t value =
			std::is_same_v<T, std::variant_alternative_t<I - 1, DecayedVariantType>> ? I - 1 : variant_type_index_impl<T, DecayedVariantType, I - 1>::value;
	};
	template<class T, class VariantType>
	class variant_type_index_impl<T, VariantType, 0>
	{
		using DecayedVariantType = std::decay_t<VariantType>;
	public:
		static constexpr size_t value = std::is_same_v<T, std::variant_alternative_t<0, DecayedVariantType>> ? 0 : std::numeric_limits<size_t>::max();
	};

	template<class... Args, size_t... Indices>
	inline const void* get_unsafe_impl(const void** ptrs, const std::variant<Args...>& variant, std::integer_sequence<size_t, Indices...> seq)
	{
		const void* ptrArray[] = { (variant.index() == Indices ? &std::get<Indices>(variant) : nullptr)... };
		for (size_t i = 0; i < sizeof...(Args); i++)
		{
			if (ptrArray[i])
				return ptrArray[i];
		}

		throw std::runtime_error();
	}
}

template<class T, class VariantType>
class variant_type_index : public std::integral_constant<size_t, detail::variant_type_index_impl<T, VariantType, std::variant_size_v<std::decay_t<VariantType>>>::value>
{
	static_assert(value != std::numeric_limits<size_t>::max(), "The specified type was not in this variant.");
};

template<class T, class VariantType> constexpr size_t variant_type_index_v = variant_type_index<T, VariantType>::value;

template<class T> __forceinline std::string type_name()
{
	return typeid(T).name();
}