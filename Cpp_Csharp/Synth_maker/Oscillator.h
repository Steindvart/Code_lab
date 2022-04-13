#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include <vector>

namespace Oscillator
{
	enum Type
	{
		SINE,
		SQUARE,
		TRIANGLE,
		SAW_ANA,
		SAW_DIG,
		NOISE
	};

	static const std::vector<std::wstring> Markers
	{
		L"SINE", L"SQUARE", L"TRIANGLE",
		L"SAW_ANA", L"SAW_DIG", L"NOISE"
	};
}

#endif // OSCILLATOR_H