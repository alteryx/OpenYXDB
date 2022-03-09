#pragma once

#include "Base/Base_ImpExp.h"
#include "Base/U16unit.h"

namespace SRC {

template <typename CharType>
BASE_EXPORT unsigned ConvertToDouble(const CharType* data, double& outVal, char sep);

template <typename CharType>
BASE_EXPORT unsigned ConvertToDouble(const CharType* data, double& outVal);

template <typename CharType>
inline double ConvertToDouble(const CharType* data, char sep = '.')
{
	double outVal;
	ConvertToDouble(data, outVal, sep);
	return outVal;
}

class DoubleToString
{
public:
	// the purpose of these convert functions is to convert a float or double to a string with the following goals.
	// 1) don't display meaningless trailing 0's or long rounds offs like 40.099998 instead of 40.01
	// 2) don't use scie notation where it makes a number look less presentable
	//			i.e. 0.0123 should NOT be 1.23E-002
	//			exponential notation will be used when numbers would get unwieldy without it
	// 3) guarantee that if you round trip the string back to the same float or double that you get the same value
	//			Sometimes this means you wilt more digits in the string than are really significant.
	//
	// generally floats have up to 7 total significant digits and doubles 15.  Sometime it can go up to 9 & 17.
	// ConvertFloatBufferSize or ConvertDoubleBufferSize should be used for the buffer sizes for these functions
	enum : unsigned
	{
		ConvertFloatBufferSize = 20,
		ConvertDoubleBufferSize = 40
	};
	BASE_EXPORT static void Convert(char* buffer, float f);
	BASE_EXPORT static void Convert(U16unit* buffer, float f);
	BASE_EXPORT static void Convert(char* buffer, double f);
	BASE_EXPORT static void Convert(U16unit* buffer, double f);
};
}  // namespace SRC
