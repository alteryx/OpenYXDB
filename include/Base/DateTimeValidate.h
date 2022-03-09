#pragma once

namespace SRC {
/** validate that str points to nDigits, and produce its value. Returns false if
	* there are not nDigits in a row. nDigits must be between 1 and 9 so this does not
	* have to check for overflow. */
template <class TChar, int nDigits>
bool IsNDigitInt(int& val, const TChar* str) noexcept
{
	unsigned v = str[0] - '0';
	if (v > 9)
		return false;
	for (int dig = 1; dig < nDigits; ++dig)
	{
		unsigned nv = str[dig] - '0';
		if (nv > 9)
			return false;
		v = v * 10 + nv;
	}
	val = v;
	return true;
}
template <typename TChar>
class TDateTimeValidate
{
public:
	static bool ValidateDate(const TChar* pVal, int nLen);
	static bool ValidateTime(const TChar* pVal, int nLen);
	static bool ValidateDateTime(const TChar* pVal, int nLen);
};
template <typename TChar>
bool TDateTimeValidate<TChar>::ValidateDate(const TChar* pVal, int nLen)
{
	int nYear, nMonth, nDay;
	if (nLen == 10 && IsNDigitInt<TChar, 4>(nYear, pVal) && pVal[4] == '-' && IsNDigitInt<TChar, 2>(nMonth, pVal + 5)
		&& pVal[7] == '-' && IsNDigitInt<TChar, 2>(nDay, pVal + 8))
	{
		if (nYear < 1400)
			return false;
		switch (nMonth)
		{
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				if (nDay > 31)
					return false;
				break;
			case 2:
				if (nDay > 29)
					return false;
				if (nDay == 29 && ((nYear % 4 != 0) || ((nYear % 100 == 0) && (nYear % 400 != 0))))
					return false;
				break;
			case 4:
			case 6:
			case 9:
			case 11:
				if (nDay > 30)
					return false;
				break;
			default:
				return false;  // invalid month number
		}
		if (nDay == 0)
			return false;

		return true;
	}

	return false;
}

template <typename TChar>
bool TDateTimeValidate<TChar>::ValidateTime(const TChar* pVal, int nLen)
{
	int val;
	if (nLen == 8 && IsNDigitInt<TChar, 2>(val, pVal) && val <= 23 && pVal[2] == ':'
		&& IsNDigitInt<TChar, 2>(val, pVal + 3) && val <= 59 && pVal[5] == ':' && IsNDigitInt<TChar, 2>(val, pVal + 6)
		&& val <= 59)
	{
		return true;
	}
	return false;
}
template <typename TChar>
bool TDateTimeValidate<TChar>::ValidateDateTime(const TChar* pVal, int nLen)
{
	if (nLen == 10)
		return ValidateDate(pVal, 10);
	else if (nLen == 19)
		return ValidateDate(pVal, 10) && pVal[10] == ' ' && ValidateTime(pVal + 11, 8);
	else
		return false;
}
}  // namespace SRC
