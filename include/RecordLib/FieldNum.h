// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

#pragma once

#include <cmath>

#include <float.h>

#include "FieldBase.h"

namespace SRC {
// the 32bit compiler likes to optimize away this test
#ifndef __GNUG__
	#pragma optimize("", off)
#endif
// returns true if the double can NOT fit the int64

#pragma warning(push)
#pragma warning(disable : 4127)  // conditional expression is constant

template <class TFloat, class TInt>
inline bool TestIntToFloat(TInt)
{
	return false;
}

template <>
inline bool TestIntToFloat<float, int64_t>(int64_t x)
{
	return x > (1ll << (FLT_MANT_DIG - 1)) || x < -(1ll << (FLT_MANT_DIG - 1));
}

template <>
inline bool TestIntToFloat<double, int64_t>(int64_t x)
{
	return x > (1ll << (DBL_MANT_DIG - 1)) || x < -(1ll << (DBL_MANT_DIG - 1));
}

template <>
inline bool TestIntToFloat<double, int>(int)
{
	return false;
}

template <>
inline bool TestIntToFloat<float, int>(int x)
{
	return x > (1 << (FLT_MANT_DIG - 1)) || x < -(1 << (FLT_MANT_DIG - 1));
}

#ifndef __GNUG__
	#pragma optimize("", on)
#endif

template <class TChar>
inline void ForceNullTerminated(Tstr<TChar>& buffer, const TChar*& pVal, size_t nLen)
{
	if (pVal[nLen] != 0)
	{
		buffer.Truncate(0);
		buffer.Append(pVal, unsigned(nLen));
		pVal = buffer;
	}
}

template <int ft, class T_Num>
class RECORDLIB_EXPORT_CPP Field_Num : public FieldBase
{
protected:
	inline TFieldVal<T_Num> GetVal(const RecordData* pRecord) const
	{
		if (*(ToCharP(pRecord) + GetOffset() + sizeof(T_Num)))
			return TFieldVal<T_Num>(true, 0);

		return TFieldVal<T_Num>(false, *reinterpret_cast<const T_Num*>(ToCharP(pRecord) + GetOffset()));
	}

	inline void SetVal(Record* pRecord, T_Num val) const
	{
		memcpy(ToCharP(pRecord->GetRecord()) + GetOffset(), &val, sizeof(T_Num));

		// and set the NULL flag
		*(ToCharP(pRecord->GetRecord()) + GetOffset() + sizeof(T_Num)) = 0;
	}

	template <class TNum>
	inline void NumericConversionError(TNum n, const String::TChar* pDestType = NULL) const
	{
		if (m_pGenericEngine)
		{
			ReportFieldConversionError(XMSG(
				"@1 does not fit in the type @2",
				String(n),
				(pDestType == NULL) ? ConvertToString(GetNameFromFieldType(m_ft)).c_str() : pDestType));
		}
	}

	template <class TChar>
	inline void CheckStringConvError(Record* pRecord, const TChar* pVal, size_t nLen, const TChar* pEnd) const
	{
		if (errno == ERANGE)
		{
			SetNull(pRecord);
			if (m_pGenericEngine)  // this gets set to NULL when the limit has been hit, so no point in adding the strings in the error message
			{
				if (sizeof(T_Num) == 8)
					ReportFieldConversionError(XMSG("@1 does not fit in an Int64.", ConvertToWString(pVal)));
				else
					ReportFieldConversionError(XMSG("@1 does not fit in an Int32.", ConvertToWString(pVal)));
			}
		}

		if (pEnd != (pVal + nLen) || nLen == 0)
		{
			if (pEnd == pVal || nLen == 0)
			{
				SetNull(pRecord);
				if (nLen > 0 && m_pGenericEngine)
					ReportFieldConversionError(XMSG("@1 is not a number.", ConvertToWString(pVal)));
			}
			else if (*pEnd == ',')
			{
				if (m_pGenericEngine)
					ReportFieldConversionError(
						XMSG("@1 stopped converting at a comma. It might be invalid.", ConvertToWString(pVal)));
			}
			else
			{
				if (m_pGenericEngine)
					ReportFieldConversionError(XMSG("@1 was not fully converted", ConvertToWString(pVal)));
			}
		}
	}

public:
	inline Field_Num(String strFieldName, int nScale = -1)
		: FieldBase(strFieldName, static_cast<E_FieldType>(ft), sizeof(T_Num) + 1, false, sizeof(T_Num), nScale)
	{
	}

	inline Field_Num(const FieldSchema& fieldSchema, int nScale = -1)
		: FieldBase(
			{ fieldSchema, fieldSchema.GetFieldName(), static_cast<E_FieldType>(ft), sizeof(T_Num), nScale },
			sizeof(T_Num) + 1,
			false)
	{
	}

	virtual SmartPointerRefObj<FieldBase> Copy() const
	{
		return CopyHelper(new Field_Num<ft, T_Num>(GetFieldName(), m_nScale));
	}

	virtual TFieldVal<bool> GetAsBool(const RecordData* pRecord) const
	{
		TFieldVal<T_Num> val = GetVal(pRecord);
		TFieldVal<bool> ret(val.bIsNull, false);
		if (!val.bIsNull)
			ret.value = Field_Bool::ConvertNumber(val.value);

		return ret;
	}

	virtual TFieldVal<int> GetAsInt32(const RecordData* pRecord) const
	{
		TFieldVal<T_Num> val = GetVal(pRecord);
		if ((!std::numeric_limits<T_Num>::is_integer || sizeof(T_Num) > sizeof(int))
			&& (val.value > T_Num(std::numeric_limits<int>::max())
				|| val.value < T_Num(std::numeric_limits<int>::min())))
		{
			NumericConversionError(val.value, _U("Int32"));
			val.bIsNull = true;
			val.value = 0;
		}
		if (!std::numeric_limits<T_Num>::is_integer)
			return TFieldVal<int>(val.bIsNull, static_cast<int>(val.value + (val.value < 0 ? -0.5 : 0.5)));
		else
			return TFieldVal<int>(val.bIsNull, static_cast<int>(val.value));
	}

	virtual TFieldVal<int64_t> GetAsInt64(const RecordData* pRecord) const
	{
		TFieldVal<T_Num> val = GetVal(pRecord);
		if (!std::numeric_limits<T_Num>::is_integer
			&& (val.value > std::numeric_limits<int64_t>::max() || val.value < std::numeric_limits<int64_t>::min()))
		{
			NumericConversionError(val.value, _U("Int64"));
			val.bIsNull = true;
			val.value = 0;
		}
		if (!std::numeric_limits<T_Num>::is_integer)
			return TFieldVal<int64_t>(val.bIsNull, static_cast<int64_t>(val.value + (val.value < 0 ? -0.5 : 0.5)));
		else
			return TFieldVal<int64_t>(val.bIsNull, static_cast<int64_t>(val.value));
	}

	virtual TFieldVal<double> GetAsDouble(const RecordData* pRecord) const
	{
		TFieldVal<T_Num> val = GetVal(pRecord);
		if (std::numeric_limits<T_Num>::is_integer && TestIntToFloat<double>(val.value))
			NumericConversionError(val.value, _U("Double"));
		return TFieldVal<double>(val.bIsNull, static_cast<double>(val.value));
	}

	virtual TFieldVal<AStringVal> GetAsAString(const RecordData* pRecord) const
	{
		TFieldVal<T_Num> val = GetVal(pRecord);
		if (val.bIsNull)
			m_astrTemp.Truncate(0);
		else
			m_astrTemp.Assign(val.value);
		TFieldVal<AStringVal> ret(val.bIsNull, AStringVal(m_astrTemp.Length(), m_astrTemp.c_str()));
		return ret;
	}

	virtual TFieldVal<WStringVal> GetAsWString(const RecordData* pRecord) const
	{
		TFieldVal<T_Num> val = GetVal(pRecord);
		if (val.bIsNull)
			m_wstrTemp.Truncate(0);
		else
			m_wstrTemp.Assign(val.value);
		TFieldVal<WStringVal> ret(val.bIsNull, WStringVal(m_wstrTemp.Length(), m_wstrTemp.c_str()));
		return ret;
	}

	virtual void SetFromInt32(Record* pRecord, int nVal) const
	{
		if (sizeof(T_Num) < sizeof(int)
			&& (nVal > std::numeric_limits<T_Num>::max() || nVal < std::numeric_limits<T_Num>::min()))
		{
			NumericConversionError(nVal);
			SetNull(pRecord);
		}
		else
		{
			if (!std::numeric_limits<T_Num>::is_integer && TestIntToFloat<T_Num>(nVal))
				NumericConversionError(nVal);
			SetVal(pRecord, static_cast<T_Num>(nVal));
		}
	}

	virtual void SetFromInt64(Record* pRecord, int64_t nVal) const
	{
		if (sizeof(T_Num) < sizeof(int64_t) && std::numeric_limits<T_Num>::is_integer
			&& (nVal > std::numeric_limits<T_Num>::max() || nVal < std::numeric_limits<T_Num>::min()))
		{
			NumericConversionError(nVal);
			SetNull(pRecord);
		}
		else
		{
			if (!std::numeric_limits<T_Num>::is_integer && TestIntToFloat<T_Num>(nVal))
				NumericConversionError(nVal);

			SetVal(pRecord, static_cast<T_Num>(nVal));
		}
	}

	virtual void SetFromDouble(Record* pRecord, double dVal) const
	{
		if (std::numeric_limits<T_Num>::is_integer
			&& (dVal > std::numeric_limits<T_Num>::max() || dVal < std::numeric_limits<T_Num>::min()))
		{
			NumericConversionError(dVal);
			SetNull(pRecord);
		}
		else
		{
			if (std::numeric_limits<T_Num>::is_integer)
				dVal += dVal < 0 ? -0.5 : 0.5;
			SetVal(pRecord, static_cast<T_Num>(dVal));
		}
	}

	virtual void SetFromString(Record* pRecord, const char* pVal, size_t nLen) const
	{
		ForceNullTerminated<char>(m_astrTemp, pVal, nLen);

		errno = 0;
		char* pEnd;
		if (sizeof(T_Num) == 8)
			SetVal(pRecord, static_cast<T_Num>(StringHelper::sh_strtoi64(pVal, &pEnd, 10)));
		else
			SetVal(pRecord, static_cast<T_Num>(StringHelper::sh_strtoi(pVal, &pEnd, 10)));

		CheckStringConvError(pRecord, pVal, nLen, pEnd);
	}

	virtual void SetFromString(Record* pRecord, const U16unit* pVal, size_t nLen) const
	{
		// TODO e2 - this might be a performance hit - but probably very rare
		WString temp;
		ForceNullTerminated<U16unit>(temp, pVal, nLen);

		errno = 0;
		U16unit* pEnd;
		if (sizeof(T_Num) == 8)
			SetVal(pRecord, static_cast<T_Num>(StringHelper::sh_strtoi64(pVal, &pEnd, 10)));
		else
			SetVal(pRecord, static_cast<T_Num>(StringHelper::sh_strtoi(pVal, &pEnd, 10)));

		CheckStringConvError(pRecord, pVal, nLen, pEnd);
	}

	virtual bool GetNull(const RecordData* pRecord) const
	{
		return *(ToCharP(pRecord) + GetOffset() + sizeof(T_Num)) != 0;
	}

	virtual void SetNull(Record* pRecord) const
	{
		memset(ToCharP(pRecord->GetRecord()) + GetOffset(), 0, sizeof(T_Num));

		// and set the NULL flag
		*(ToCharP(pRecord->GetRecord()) + GetOffset() + sizeof(T_Num)) = 1;
	}
};

#pragma warning(pop)

typedef Field_Num<E_FT_Byte, unsigned char> Field_Byte;
typedef Field_Num<E_FT_Int16, signed short> Field_Int16;
typedef Field_Num<E_FT_Int32, signed int> Field_Int32;
typedef Field_Num<E_FT_Int64, int64_t> Field_Int64;
}  // namespace SRC
