#ifndef OCTAVES_H
#define OCTAVES_H

#include <vector>

namespace Octaves
{
	constexpr double A0_val = 27.5;
	constexpr double A1_val = A0_val * 2;
	constexpr double A2_val = A1_val * 2;
	constexpr double A3_val = A2_val * 2;
	constexpr double A4_val = A3_val * 2;
	constexpr double A5_val = A4_val * 2;
	constexpr double A6_val = A5_val * 2;
	constexpr double A7_val = A6_val * 2;
	constexpr double A8_val = A7_val * 2;

	enum Type
	{
		A0, A1, A2, A3, A4, A5, A6, A7, A8
	};
	static const std::vector<double> All{ A0_val, A1_val, A2_val, A3_val, A4_val, A5_val, A6_val, A7_val, A8_val };
}

#endif // OCTAVES_H