#pragma once
#include <any>
#include <stdexcept>

template<class T = std::runtime_error> class BaseException : public T
{
public:
	BaseException(const std::string& type, const std::string& msg, const std::any& baseException = std::any()) :
		T(msg),
		m_InnerException(baseException)
	{
		Log::Msg<LogType::Exception>("{0}: {1}", type, msg);
	}

	std::any m_InnerException;
};