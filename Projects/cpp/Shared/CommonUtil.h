#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include <string>

namespace CommonUtil
{

// #TODO: to made converters based by `std::`
std::wstring s2ws(const std::string& src);
std::wstring s2ws(const char* str);
std::wstring s2ws(const std::string_view src);

std::string ws2s(const std::wstring& src);
std::string ws2s(const wchar_t* src);
std::string ws2s(const std::wstring_view src);

} // !namespace CommonUtil

#endif // !COMMON_UTIL_H