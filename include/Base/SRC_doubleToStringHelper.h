#pragma once
#include <assert.h>

namespace SRC {
struct DoubleToStringHelper
{
private:
#ifdef _WIN32
	static const unsigned ConvertFloatBufferSize = 20;
	static const unsigned ConvertDoubleBufferSize = 40;

	template <class TNum, int MinNumDigits>
	static bool DoEcvt(char* buffer, TNum f);
#else
	template <typename T, int exp>
	struct CompileTimePow
	{
		static constexpr T i{ 10 };
		static constexpr T value{ i * CompileTimePow<T, exp - 1>::value };
	};

	template <typename T>
	struct CompileTimePow<T, 0>
	{
		static constexpr T value{ 1.0 };
	};

	template <typename T, int exp>
	struct CompileTimeNegPow
	{
		static constexpr T value{ static_cast<T>(1) / CompileTimePow<T, exp>::value };
	};

	template <int MinNumDigits>
	struct BufferSize
	{
		static constexpr unsigned ConvertFloatBufferSize{ 20 };
		static constexpr unsigned ConvertDoubleBufferSize{ 40 };

		static constexpr unsigned value{ MinNumDigits <= 7 ? ConvertFloatBufferSize : ConvertDoubleBufferSize };
	};

	template <class TNum, int MinNumDigits>
	static void DoSnprintf(char* buffer, TNum f);
#endif

	template <class TNum, int MinNumDigits>
	static void DoGcvt(char* buffer, TNum f);

public:
	template <class TNum, int MinNumDigits>
	static void Convert(char* buffer, TNum f);
};
}  // namespace SRC
