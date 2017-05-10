#pragma once

#include <map>
#include <variant>
#include <vector>

enum class JSONDataType
{
	Number,
	String,
	Bool,
	Array,
	Object
};

class JSONValue
{
public:
	JSONValue();
	~JSONValue();

private:
	union
	{
		double m_Number;
		std::string m_String;
	} m_Data;
};

class JSONObject
{
public:

private:
	std::vector<JSONValue> m_Values;
};

class JSONSerializer
{
public:


private:

};

namespace JSON
{
	namespace internal
	{
		// Dumb hack to get std::variant to contain itself
		struct PlaceholderArray final
		{
			PlaceholderArray();
			PlaceholderArray(const PlaceholderArray& other);
			PlaceholderArray(PlaceholderArray&& other);
			~PlaceholderArray();
		private:
			uint8_t placeholder[32];
		};
		struct PlaceholderObject final
		{
			PlaceholderObject();
			PlaceholderObject(const PlaceholderObject& other);
			PlaceholderObject(PlaceholderObject&& other);
			~PlaceholderObject();
		private:
			uint8_t placeholder[24];
		};

		using Variant = std::variant<double, std::string, bool, PlaceholderArray, PlaceholderObject>;

		using RealArray = std::vector<Variant>;
		constexpr auto PLACEHOLDER_ARRAY_SIZE = sizeof(PlaceholderArray);
		constexpr auto ACTUAL_ARRAY_SIZE = sizeof(RealArray);
		static_assert(PLACEHOLDER_ARRAY_SIZE == ACTUAL_ARRAY_SIZE, "Array placeholder size mismatch!");

		using RealObject = std::map<std::string, Variant>;
		constexpr auto PLACEHOLDER_OBJECT_SIZE = sizeof(PlaceholderObject);
		constexpr auto ACTUAL_OBJECT_SIZE = sizeof(RealObject);
		static_assert(PLACEHOLDER_OBJECT_SIZE == ACTUAL_OBJECT_SIZE, "Object placeholder size mismatch!");
	}

	using Array = internal::RealArray;
	using Object = internal::RealObject;
	using Value = std::variant<double, std::string, bool, Array, Object>;

	extern Value LoadFromString(const std::string& str);
}