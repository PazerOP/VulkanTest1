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

extern std::ostream& operator<<(std::ostream& lhs, JSONDataType rhs);

class JSONValue;
using JSONArray = std::vector<JSONValue>;

class JSONObject : public std::map<std::string, JSONValue>
{
public:
	double TryGetNumber(const std::string& name, double default, bool* success = nullptr) const;
	bool TryGetBool(const std::string& name, bool default, bool* success = nullptr) const;
	std::string TryGetString(const std::string& name, const std::string& default, bool* success = nullptr) const;

	const double* TryGetNumber(const std::string& name) const;
	const bool* TryGetBool(const std::string& name) const;
	const std::string* TryGetString(const std::string& name) const;
	const JSONArray* TryGetArray(const std::string& name) const;
	const JSONObject* TryGetObject(const std::string& name) const;
	const JSONValue* TryGetValue(const std::string& name) const;

	double* TryGetNumber(const std::string& name) { return const_cast<double*>(std::as_const(*this).TryGetNumber(name)); }
	bool* TryGetBool(const std::string& name) { return const_cast<bool*>(std::as_const(*this).TryGetBool(name)); }
	std::string* TryGetString(const std::string& name) { return const_cast<std::string*>(std::as_const(*this).TryGetString(name)); }
	JSONArray* TryGetArray(const std::string& name) { return const_cast<JSONArray*>(std::as_const(*this).TryGetArray(name)); }
	JSONObject* TryGetObject(const std::string& name) { return const_cast<JSONObject*>(std::as_const(*this).TryGetObject(name)); }
	JSONValue* TryGetValue(const std::string& name) { return const_cast<JSONValue*>(std::as_const(*this).TryGetValue(name)); }

	///////////////////////////////////////////////////////////////////
	// These throw json_value_missing_error or json_value_type_error //
	///////////////////////////////////////////////////////////////////
	const double& GetNumber(const std::string& name) const;
	const bool& GetBool(const std::string& name) const;
	const std::string& GetString(const std::string& name) const;
	const JSONArray& GetArray(const std::string& name) const;
	const JSONObject& GetObject(const std::string& name) const;
	const JSONValue& GetValue(const std::string& name) const;

	double& GetNumber(const std::string& name) { return const_cast<double&>(std::as_const(*this).GetNumber(name)); }
	std::string& GetString(const std::string& name) { return const_cast<std::string&>(std::as_const(*this).GetString(name)); }
	bool& GetBool(const std::string& name) { return const_cast<bool&>(std::as_const(*this).GetBool(name)); }
	JSONArray& GetArray(const std::string& name) { return const_cast<JSONArray&>(std::as_const(*this).GetArray(name)); }
	JSONObject& GetObject(const std::string& name) { return const_cast<JSONObject&>(std::as_const(*this).GetObject(name)); }
	JSONValue& GetValue(const std::string& name) { return const_cast<JSONValue&>(std::as_const(*this).GetValue(name)); }
};

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

	const double& GetNumber() const;
	double& GetNumber() { return const_cast<double&>(std::as_const(*this).GetNumber()); }

	const bool& GetBool() const;
	bool& GetBool() { return const_cast<bool&>(std::as_const(*this).GetBool()); }

	const std::string& GetString() const;
	std::string& GetString() { return const_cast<std::string&>(std::as_const(*this).GetString()); }

	const JSONArray& GetArray() const;
	JSONArray& GetArray() { return const_cast<JSONArray&>(std::as_const(*this).GetArray()); }

	const JSONObject& GetObject() const;
	JSONObject& GetObject() { return const_cast<JSONObject&>(std::as_const(*this).GetObject()); }

	JSONDataType GetType() const;

private:
	std::variant<std::monostate, double, std::string, bool, JSONArray, JSONObject> m_Data;
};
class json_error : public std::runtime_error
{
public:
	json_error(const std::string& type, const std::string& msg) : std::runtime_error(msg)
	{
		Log::Msg<LogType::Exception>(StringTools::CSFormat("{0}: {1}", type, msg));
	}
	json_error(const std::string& msg) : json_error(typeid(*this).name(), msg) { }
};
class json_parsing_error : public json_error
{
public:
	json_parsing_error(const std::string& msg) : json_error(typeid(*this).name(), msg) { }
};
class json_value_missing_error : public json_error
{
public:
	json_value_missing_error(const std::string& msg) : json_error(typeid(*this).name(), msg) { }
};
class json_value_type_error : public json_error
{
public:
	json_value_type_error(const std::string& msg) : json_error(typeid(*this).name(), msg) { }
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