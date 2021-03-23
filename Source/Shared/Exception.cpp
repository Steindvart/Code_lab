#include "Exception.h"

#include "CommonUtil.h"

namespace Shared
{

Exception::Exception(const std::wstring_view& strReason)
	: std::exception(CommonUtil::ws2s(strReason).c_str())
	, m_reason(strReason)
{
}

Exception::Exception(const std::string& strReason)
	: std::exception(strReason.c_str())
	, m_reason(CommonUtil::s2ws(strReason))
{
}

std::wstring Exception::Reason() const
{
	return m_reason;
}

}