#include "stdafx.h"

#include "SRC_stringHelper.h"

namespace SRC { namespace StringHelper {

namespace {

// this table is 256 bytes long so we don't have to check if a
// byte is out of range.
// clang-format off
constexpr inline int8_t CharVals[]{
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  // 00
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  // 10
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  // 20
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  99, 99, 99, 99, 99, 99,  // '0' ... '9',
	99, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  // @, A ... O
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 99, 99, 99, 99, 99,  // P ... Z
	99, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  // @, a ... o
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 99, 99, 99, 99, 99,  // p ... z
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
};
// clang-format on

}  // namespace

template <typename ChT, typename ValT>
ValT TstrtoNum(const ChT* nptr, ChT const** endptr, int base) noexcept
{
	static_assert(sizeof(CharVals) >= 0x7b, "Table needs to go from 0 to at least 'z'");
	static_assert(CharVals['a'] == 10, "Value of a is 10");
	static_assert(CharVals['z'] == 35, "Value of z is 35, top value in base 36");
	static_assert(CharVals['A'] == 10, "Value of A is 10");
	static_assert(CharVals['Z'] == 35, "Value of Z is 35, top value in base 36");
	static_assert(CharVals['0'] == 0, "Value of 0 is 0");
	static_assert(CharVals['9'] == 9, "Value of 9 is 9");
	while (CType::IsSpace(*nptr) || *nptr == '\240')
		++nptr;
	bool neg = false;
	if (*nptr == '-')
	{
		neg = true;
		++nptr;
	}
	else if (*nptr == '+')
	{
		++nptr;
	}
	if ((base == 0 || base == 16) && nptr[0] == '0' && CType::ToLowerASCII(nptr[1]) == 'x')
	{
		base = 16;
		nptr += 2;
	}
	else if ((base == 0 || base == 8) && nptr[0] == '0')
	{
		base = 8;
		++nptr;
	}
	else if (base == 0)
	{
		base = 10;
	}
	if (base < 2 || 36 < base)
	{
		errno = ERANGE;
		return 0;
	}
	// note this value is negative
	ValT lim = std::numeric_limits<ValT>::min() / base;
	// positive value of the last digit we could include
	int limDigit = static_cast<unsigned>(base * lim - std::numeric_limits<ValT>::min());
	ValT ret{ 0 };
	int digit;
	// a normal iteration through this loop will test only three conditions:
	//    the next code unit is in the table
	//    the value of the next code unit is within the desired base
	//    the return value is way less than the limit
	// even testing for the null termination is just testing that CharVals[0]>=base. Sweet!
	for (; static_cast<unsigned>(*nptr) < sizeof(CharVals) && (digit = CharVals[static_cast<unsigned>(*nptr)]) < base;
		 ++nptr)
	{
		if (ret <= lim && (ret < lim || digit > limDigit))
		{
			errno = ERANGE;
			ret = std::numeric_limits<ValT>::max();
			break;
		}
		// note I'm subtracting, so I can naturally read INT*_MIN. I need to do funny things only for INT*_MAX
		ret = ret * base - digit;
	}
	if (endptr)
		(*endptr) = nptr;
	if (!neg)
	{
		if (ret < -std::numeric_limits<ValT>::max())
		{
			errno = ERANGE;
			return std::numeric_limits<ValT>::max();
		}
		ret = -ret;
	}
	return ret;
}

template int32_t TstrtoNum<char, int32_t>(const char* nptr, char const** endptr, int base) noexcept;
template int32_t TstrtoNum<U16unit, int32_t>(const U16unit* nptr, U16unit const** endptr, int base) noexcept;

template int64_t TstrtoNum<char, int64_t>(const char* nptr, char const** endptr, int base) noexcept;
template int64_t TstrtoNum<U16unit, int64_t>(const U16unit* nptr, U16unit const** endptr, int base) noexcept;

#define limitedPut(ch)                                                                                                 \
	{                                                                                                                  \
		if (ptr >= lim)                                                                                                \
			return EINVAL;                                                                                             \
		*ptr++ = ch;                                                                                                   \
	}

template <typename ChT, typename ValT>
int utowcs(ValT uval, ChT* buffer, size_t size, int radix) noexcept
{
	if (!buffer || radix < 2 || radix > 36)
		return EINVAL;
	ChT* lim = buffer + size - 1;  // reserve space for a final null
	ChT* ptr = buffer;
	for (;;)
	{
		int digit = uval % radix;
		uval = uval / radix;
		char cdigit;
		if (digit <= 9)
			cdigit = static_cast<char>(digit + '0');
		else
			cdigit = static_cast<char>(digit + ('a' - 10));
		limitedPut(cdigit);
		if (uval == 0)
			break;
	}
	ptr[0] = '\0';
	assert(ptr <= lim);  // because limitedPut would have stopped me
	std::reverse(buffer, ptr);
	return 0;
}

template int utowcs<char, uint32_t>(uint32_t uval, char* buffer, size_t size, int radix) noexcept;
template int utowcs<U16unit, uint32_t>(uint32_t uval, U16unit* buffer, size_t size, int radix) noexcept;

template int utowcs<char, uint64_t>(uint64_t uval, char* buffer, size_t size, int radix) noexcept;
template int utowcs<U16unit, uint64_t>(uint64_t uval, U16unit* buffer, size_t size, int radix) noexcept;

template <typename ChT, typename ValT>
int itowcs(ValT value, ChT* buffer, size_t size, int radix) noexcept
{
	if (!buffer)
		return EINVAL;
	typename std::make_unsigned<ValT>::type uval = value;
#pragma warning(disable : 4127)  // conditional expression is constant
	if (value < 0 && radix == 10)
	{
		if (size < 2)
			return EINVAL;
		*buffer++ = '-';
		--size;
		uval = -value;
	}
	return utowcs(uval, buffer, size, radix);
}

template int itowcs<char, int>(int value, char* buffer, size_t size, int radix) noexcept;
template int itowcs<U16unit, int>(int value, U16unit* buffer, size_t size, int radix) noexcept;

template int itowcs<char, int64_t>(int64_t value, char* buffer, size_t size, int radix) noexcept;
template int itowcs<U16unit, int64_t>(int64_t value, U16unit* buffer, size_t size, int radix) noexcept;

int sh_strtoi(const char* nptr, char** endptr, int base) noexcept
{
	return TstrtoNum<char, int>(nptr, const_cast<const char**>(endptr), base);
}

int sh_strtoi(const U16unit* nptr, U16unit** endptr, int base) noexcept
{
	return TstrtoNum<U16unit, int>(nptr, const_cast<const U16unit**>(endptr), base);
}

int32_t sh_strtoi(const char* pBuffer) noexcept
{
	return sh_strtoi(pBuffer, nullptr, 10);
}

int32_t sh_strtoi(const U16unit* pBuffer) noexcept
{
	return sh_strtoi(pBuffer, nullptr, 10);
}

int64_t sh_strtoi64(const char* nptr, char** endptr, int base) noexcept
{
	return TstrtoNum<char, int64_t>(nptr, const_cast<const char**>(endptr), base);
}

int64_t sh_strtoi64(const U16unit* nptr, U16unit** endptr, int base) noexcept
{
	return TstrtoNum<U16unit, int64_t>(nptr, const_cast<const U16unit**>(endptr), base);
}

int64_t sh_strtoi64(const char* ptr) noexcept
{
	return sh_strtoi64(ptr, nullptr, 10);
}

int64_t sh_strtoi64(const U16unit* ptr) noexcept
{
	return sh_strtoi64(ptr, nullptr, 10);
}

int sh_itostr(int value, char* buffer, size_t size) noexcept
{
	return itowcs<char, int>(value, buffer, size, 10);
}

int sh_itostr(int value, U16unit* buffer, size_t size) noexcept
{
	return itowcs<U16unit, int>(value, buffer, size, 10);
}

int sh_i64tostr(int64_t n, char* pBuffer, size_t nSize) noexcept
{
	return itowcs<char, int64_t>(n, pBuffer, nSize, 10);
}
int sh_i64tostr(int64_t n, U16unit* pBuffer, size_t nSize) noexcept
{
	return itowcs<U16unit, int64_t>(n, pBuffer, nSize, 10);
}

int sh_dtostr(char* pBuffer, size_t nSize, const int iDecPlaces, const double d) noexcept
{
	return snprintf(pBuffer, nSize, "%.*f", iDecPlaces, d);
}

int sh_dtostr(U16unit* pBuffer, size_t nSize, const int iDecPlaces, const double d) noexcept
{
#ifdef _WIN32
	return swprintf(pBuffer, nSize, U16("%.*f"), iDecPlaces, d);
#else
	// standard does not have a printf to UTF-16, but we just need stretch out the
	// chars to char16s.
	assert(0 < nSize && nSize <= 512);  // precondition
	char cbuffer[512];
	size_t ret = snprintf(cbuffer, nSize, "%.*f", iDecPlaces, d);
	for (int j = std::min(ret, nSize); j >= 0; --j)
		pBuffer[j] = cbuffer[j];  // chars to U16unit; just (implicitly) cast chars
	return ret;
#endif
}

}}  // namespace SRC::StringHelper
