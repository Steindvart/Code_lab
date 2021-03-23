#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>

namespace Shared
{

class Exception : public std::exception
{
public:
	explicit Exception(const std::wstring_view& strReason);
	explicit Exception(const std::string& strReason);

	std::wstring Reason() const;

private:
	std::wstring m_reason;
};

} // !namespace Shared

#endif // !EXCEPTION_H
