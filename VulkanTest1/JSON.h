#pragma once

#include <filesystem>
#include <map>
#include <optional>
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

class JSONValue;
using JSONArray = std::vector<JSONValue>;
using JSONObject = std::map<std::string, JSONValue>;

class JSONValue
{
public:
	JSONValue() = default;
	explicit JSONValue(double number) : m_Data(number) { }
	explicit JSONValue(const std::string& str) : m_Data(str) { }
	explicit JSONValue(bool boolean) : m_Data(boolean) { }
	explicit JSONValue(const JSONArray& array) : m_Data(array) { }
	explicit JSONValue(const JSONObject& object) : m_Data(object) { }

	void Set(double number) { m_Data = number; }
	void Set(const std::string& str) { m_Data.emplace<std::string>(str); }
	void Set(bool boolean) { m_Data = boolean; }
	void Set(const JSONArray& array) { m_Data = array; }
	void Set(const JSONObject& object) { m_Data = object; }

	double GetNumber() const { return std::get<double>(m_Data); }
	bool GetBool() const { return std::get<bool>(m_Data); }

	const std::string& GetString() const { return std::get<std::string>(m_Data); }
	std::string& GetString() { return std::get<std::string>(m_Data); }

	const JSONArray& GetArray() const { return std::get<JSONArray>(m_Data); }
	JSONArray& GetArray() { return std::get<JSONArray>(m_Data); }

	const JSONObject& GetObject() const { return std::get<JSONObject>(m_Data); }
	JSONObject& GetObject() { return std::get<JSONObject>(m_Data); }

private:
	std::variant<double, std::string, bool, JSONArray, JSONObject> m_Data;
};

class json_parsing_error : public std::runtime_error
{
public:
	json_parsing_error(const char* msg) : std::runtime_error(msg) { }
	json_parsing_error(const std::string& msg) : std::runtime_error(msg) { }
};

class JSONSerializer
{
public:
	static JSONValue FromFile(const std::filesystem::path& path);
	static JSONValue FromString(const std::string& str);

private:
	static constexpr char ESCAPE_CHAR = '\\';

	static bool IsEscaped(const char* input);
	static bool IsNumber(char input);
	static void EatWhitespace(const char** input);

	static JSONObject GatherObject(const char** input);
	static JSONArray GatherArray(const char** input);
	static JSONValue GatherValue(const char** input);

	static double GatherNumber(const char** input);
	static std::string GatherString(const char** input);
	static std::pair<std::string, JSONValue> GatherNamedValue(const char** input);
};