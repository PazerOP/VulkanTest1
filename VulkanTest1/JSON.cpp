#include "stdafx.h"
#include "JSON.h"

#include "Enums.h"

#include <fstream>

JSONValue JSONSerializer::FromFile(const std::filesystem::path& path)
{
	std::ifstream stream(path.string(), std::ios::ate | std::ios::binary);

	const auto length = stream.tellg();
	std::string str;
	str.resize(length);
	stream.seekg(0);
	stream.read(str.data(), length);
	return FromString(str);
}

JSONValue JSONSerializer::FromString(const std::string& str)
{
	const char* pStr = str.c_str();

	if (pStr[0] == ESCAPE_CHAR)
		throw json_parsing_error(StringTools::CSFormat("String begins with ESCAPE_CHAR ({0})", ESCAPE_CHAR));

	while (isspace(*pStr))
		pStr++;

	if (*pStr == '{')
		return JSONValue(GatherObject(&pStr));
	else if (*pStr == '[')
		return JSONValue(GatherArray(&pStr));
	else if (*pStr == '"')
		return GatherValue(&pStr);
	else
		throw json_parsing_error(StringTools::CSFormat("Encountered unknown character '{0}'", *pStr));
}

bool JSONSerializer::IsEscaped(const char* input)
{
	// This will overrun if the string begins with an escape char, but we explicitly check for
	// that case in JsonSerializer::FromString.
	size_t escapeCharCount = 0;
	while (*(--input) == ESCAPE_CHAR)
		escapeCharCount++;

	return (escapeCharCount % 2) == 1;
}

bool JSONSerializer::IsNumber(char input)
{
	assert(input != '\0');
	return isdigit(input) || input == '+' || input == '-' || input == '.';
}

void JSONSerializer::EatWhitespace(const char** input)
{
	while (isspace(**input))
		(*input)++;
}

JSONObject JSONSerializer::GatherObject(const char** input)
{
	assert(**input == '{');
	(*input)++;

	JSONObject retVal;

	std::string nameBuf;
	while (**input != '\0')
	{
		EatWhitespace(input);

		if (**input == '"')
			retVal.insert(GatherNamedValue(input));
		//else if (IsNumber(**input))
		//	JSONAddValue(objOut, GatherNumber(input));
		else if (**input == '}')
		{
			(*input)++;
			if (**input == ',')
				(*input)++;

			break;
		}
		else
			throw json_parsing_error(StringTools::CSFormat("Malformed JSON file! Expected '\"' or '}', but encountered '{0}'.", **input));
	}
	return retVal;
}

JSONArray JSONSerializer::GatherArray(const char ** input)
{
	assert(**input == '[');
	(*input)++;

	JSONArray retVal;
	while (**input != '\0')
	{
		EatWhitespace(input);

		if (**input == '"')
			retVal.push_back(GatherValue(input));
		else if (**input == '{')
			retVal.push_back(JSONValue(GatherObject(input)));
		else if (**input == '[')
			retVal.push_back(JSONValue(GatherArray(input)));
		else if (IsNumber(**input))
			retVal.push_back(JSONValue(GatherNumber(input)));
		else if (**input == ']')
		{
			(*input)++;
			if (**input == ',')
				(*input)++;

			break;
		}
		else
			throw json_parsing_error(StringTools::CSFormat("Malformed JSON file"));
	}

	return retVal;;
}

JSONValue JSONSerializer::GatherValue(const char** input)
{
	JSONValue retVal;

	EatWhitespace(input);

	if (**input == '"')
		retVal.Set(GatherString(input));
	else if (**input == '{')
		retVal.Set(GatherObject(input));
	else if (**input == '[')
		retVal.Set(GatherArray(input));
	else if (IsNumber(**input))
		retVal.Set(GatherNumber(input));
	else if (!strncmp(*input, "true", 4) || !strncmp(*input, "false", 5))
	{
		if (**input == 't')
		{
			retVal.Set(true);
			(*input) += 4;	// "true"
		}
		else
		{
			retVal.Set(false);
			(*input) += 5;	// "false"
		}
	}
	else
		throw json_parsing_error("Malformed JSON file");

	if (**input == ',')
		(*input)++;

	return retVal;
}

double JSONSerializer::GatherNumber(const char** input)
{
	char* endPtr;
	const double retVal = strtod(*input, &endPtr);
	assert(endPtr > *input);
	*input = endPtr;

	if (**input == ',')
		(*input)++;

	return retVal;
}

std::string JSONSerializer::GatherString(const char** input)
{
	assert(**input == '"');
	(*input)++;

	std::stringstream retVal;
	bool nonSlashGathered = false;

	while ((**input != '"' || IsEscaped(*input)) && **input != '\0')
	{
		const bool escaped = IsEscaped(*input);
		char c = **input;
		if (c != ESCAPE_CHAR || escaped)
		{
			if (c != ESCAPE_CHAR)
				nonSlashGathered = true;
			if (escaped && c == 'n')
				c = '\n';

			retVal << c;
		}

#if 0
		if (charsGathered > 0 && nonSlashGathered && IsEscaped(output))
		{
			charsGathered--;
			maxLength--;
			output--;

			// Escaped control characters
			if (c == 'n')
				*output = '\n';
			else
				continue;
		}
#endif

		(*input)++;
	}

	// Closing quote
	assert(**input == '"');
	(*input)++;

	return retVal.str();
}

std::pair<std::string, JSONValue> JSONSerializer::GatherNamedValue(const char** input)
{
	assert(**input == '"');
	auto name = GatherString(input);

	if (**input != ':')
		throw json_parsing_error("Malformed JSON file");
	else
		(*input)++;

	auto value = GatherValue(input);
	return std::make_pair(std::move(name), std::move(value));
}

std::ostream& operator<<(std::ostream& lhs, JSONDataType rhs)
{
	switch (rhs)
	{
	case JSONDataType::Number:		return lhs << "JSONDataType::Number"sv;
	case JSONDataType::String:		return lhs << "JSONDataType::String"sv;
	case JSONDataType::Bool:		return lhs << "JSONDataType::Bool"sv;
	case JSONDataType::Array:		return lhs << "JSONDataType::Array"sv;
	case JSONDataType::Object:		return lhs << "JSONDataType::Object"sv;

	default:
		return lhs << StringTools::CSFormat("<*** UNKNOWN JSONDataType {0} ***>", Enums::value(rhs));
	}
}

double JSONObject::TryGetNumber(const std::string& name, double default, bool* success) const
{
	const auto& found = find(name);
	if (found == end())
	{
		if (success)
			*success = false;

		return default;
	}

	if (success)
		*success = true;

	return found->second.GetNumber();
}

bool JSONObject::TryGetBool(const std::string& name, bool default, bool* success) const
{
	const auto& found = find(name);
	if (found == end())
	{
		if (success)
			*success = false;

		return default;
	}

	if (success)
		*success = true;

	return found->second.GetBool();
}

std::string JSONObject::TryGetString(const std::string& name, const std::string& default, bool* success) const
{
	const auto& found = find(name);
	if (found == end())
	{
		if (success)
			*success = false;

		return default;
	}

	if (success)
		*success = true;

	return found->second.GetString();
}

std::optional<double> JSONObject::TryGetNumber(const std::string& name) const
{
	const auto& found = find(name);
	if (found == end())
		return std::nullopt;

	return found->second.GetNumber();
}

const double& JSONObject::GetNumber(const std::string& name) const
{
	const auto& found = find(name);
	if (found == end())
		throw json_error(StringTools::CSFormat("Missing required number value {0}", name));

	return found->second.GetNumber();
}

std::optional<bool> JSONObject::TryGetBool(const std::string& name) const
{
	const auto& found = find(name);
	if (found == end())
		return std::nullopt;

	return found->second.GetBool();
}

const bool& JSONObject::GetBool(const std::string& name) const
{
	const auto& found = find(name);
	if (found == end())
		throw json_error(StringTools::CSFormat("Missing required boolean value {0}", name));

	return found->second.GetBool();
}

std::optional<std::string> JSONObject::TryGetString(const std::string& name) const
{
	const auto& found = find(name);
	if (found == end())
		return std::nullopt;

	return found->second.GetString();
}

const std::string& JSONObject::GetString(const std::string& name) const
{
	const auto& found = find(name);
	if (found == end())
		throw json_error(StringTools::CSFormat("Missing required string value {0}", name));

	return found->second.GetString();
}

const JSONArray& JSONObject::GetArray(const std::string& name) const
{
	const auto& found = find(name);
	if (found == end())
		throw json_error(StringTools::CSFormat("Missing required array value {0}", name));

	return found->second.GetArray();
}

const JSONObject& JSONObject::GetObject(const std::string& name) const
{
	const auto& found = find(name);
	if (found == end())
		throw json_error(StringTools::CSFormat("Missing required object value {0}", name));

	return found->second.GetObject();
}

const double& JSONValue::GetNumber() const
{
	if (GetType() != JSONDataType::Number)
		throw json_value_type_error(StringTools::CSFormat("Attempted to call {0}(), but data type on was {1}.", __FUNCTION__, GetType()));

	return std::get<double>(m_Data);
}

const bool& JSONValue::GetBool() const
{
	if (GetType() != JSONDataType::Bool)
		throw json_value_type_error(StringTools::CSFormat("Attempted to call {0}(), but data type on was {1}.", __FUNCTION__, GetType()));

	return std::get<bool>(m_Data);
}

const std::string& JSONValue::GetString() const
{
	if (GetType() != JSONDataType::String)
		throw json_value_type_error(StringTools::CSFormat("Attempted to call {0}(), but data type on was {1}.", __FUNCTION__, GetType()));

	return std::get<std::string>(m_Data);
}

const JSONArray& JSONValue::GetArray() const
{
	if (GetType() != JSONDataType::Array)
		throw json_value_type_error(StringTools::CSFormat("Attempted to call {0}(), but data type on was {1}.", __FUNCTION__, GetType()));

	return std::get<JSONArray>(m_Data);
}

const JSONObject& JSONValue::GetObject() const
{
	if (GetType() != JSONDataType::Object)
		throw json_value_type_error(StringTools::CSFormat("Attempted to call {0}(), but data type on was {1}.", __FUNCTION__, GetType()));

	return std::get<JSONObject>(m_Data);
}

JSONDataType JSONValue::GetType() const
{
	if (m_Data.index() == 0)
		throw std::runtime_error("Accessing uninitialized JSONValue");

	return JSONDataType(m_Data.index() - 1);
}
