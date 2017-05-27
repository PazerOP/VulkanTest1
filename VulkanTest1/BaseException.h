#pragma once
#include <any>
#include <stdexcept>

template<class T = std::runtime_error> class BaseException : public T
{
public:
	BaseException(const std::string& msg, const std::any& baseException = std::any())
		m_InnerException(baseException),
		m_Message(msg)
	{
	}

	virtual const char* what() override { return StringTools::CSFormat("{0}: {1}", type_name(type), m_Message); }

	std::any m_InnerException;
	std::string m_Message;
};