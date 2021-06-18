#include "CommonUtil.h"

#include <Windows.h>
#include <stringapiset.h>

namespace CommonUtil
{

#define INVALID_CHAR "_"
#define INVALID_WCHAR L"_"

std::wstring s2ws(const std::string& src)
{
	if (src.empty())
		return L"";

	int requiredSize = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, NULL, 0);

	//#TODO: correct str constructors
	//std::wstring result(requiredSize - 1, INVALID_WCHAR);
	std::wstring result(INVALID_WCHAR);
	result.reserve(requiredSize);

	MultiByteToWideChar(CP_UTF8, 0, src.c_str(), -1, &result[0], requiredSize);

	return result;
}

std::wstring s2ws(const char* src)
{
	return src ? s2ws(std::string(src)) : L"";
}

std::wstring s2ws(const std::string_view src)
{
	return s2ws(src.data());
}

std::string ws2s(const std::wstring& src)
{
	if (src.empty())
		return "";

	int requiredSize = WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, NULL, 0, NULL, NULL);

	// #TODO: to made correct str constructors
	//std::string result(requiredSize - 1, INVALID_CHAR);
	std::string result(INVALID_CHAR);
	result.reserve(requiredSize);

	WideCharToMultiByte(CP_UTF8, 0, src.c_str(), -1, &result[0], requiredSize, NULL, NULL);

	return result;
}

std::string ws2s(const wchar_t* src)
{
	return src ? ws2s(std::wstring(src)) : "";
}

std::string ws2s(const std::wstring_view src)
{
	return ws2s(src.data());
}

} // !namespace CommonUtil