#include "stdafx.h"

#include "RecordLib/FieldFixedDecimal.h"

#if defined(SRCLIB_REPLACEMENT) || defined(E2)
namespace {
bool IsDigit(int c)
{
	return c <= '9' && c >= '0';
}
}  // namespace
#else
	#include "Base/SRC_Algorithms.h"
#endif

namespace SRC {
/*virtual*/ TFieldVal<bool> Field_FixedDecimal::GetAsBool(const RecordData* pRecord) const
{
	// this will NOT be null terminated;
	TFieldVal<BlobVal> ret = GetAsBlob(pRecord);
	if (ret.bIsNull)
		return TFieldVal<bool>(true, false);

	// FixedDecimal derives from a narrow string
	const char* p = static_cast<const char*>(ret.value.pValue);
	for (unsigned x = 0; x < ret.value.nLength; ++x)
	{
		char c = p[x];
		if (c >= '1' && c <= '9')
			return TFieldVal<bool>(false, true);
	}
	return TFieldVal<bool>(false, false);
}

/*virtual*/ void Field_FixedDecimal::SetFromBool(Record* pRecord, bool bVal) const
{
	SetFromDouble(pRecord, bVal ? 1.0 : 0.0);
}

/*virtual*/ void Field_FixedDecimal::SetFromDouble(Record* pRecord, double dVal) const
{
	m_astrTemp.Assign(dVal, m_nScale);
	if (m_astrTemp.Length() > m_nSize)
	{
		SetNull(pRecord);
		ReportFieldConversionError(XMSG(
			"\"@1\" does not fit in Fixed Decimal @2.@3",
			ConvertToString(m_astrTemp),
			String(int(m_nSize)),
			String(int(m_nScale))));
	}
	else
		Field_String_GetSet<char>::SetVal(this, pRecord, GetOffset(), m_nSize, m_astrTemp, m_astrTemp.Length());
}

/*virtual*/ void Field_FixedDecimal::SetFromInt32(Record* pRecord, int nVal) const
{
	SetFromDouble(pRecord, double(nVal));
}

/*virtual*/ void Field_FixedDecimal::SetFromInt64(Record* pRecord, int64_t nVal) const
{
	m_astrTemp.Assign(nVal);
	if (m_astrTemp.Length() > m_nSize)
	{
		SetNull(pRecord);
		ReportFieldConversionError(XMSG(
			"\"@1\" does not fit in Fixed Decimal @2.@3",
			ConvertToString(m_astrTemp),
			String(int(m_nSize)),
			String(int(m_nScale))));
	}
	else
		SetFromString(pRecord, m_astrTemp, m_astrTemp.Length());
}

/*virtual*/ void Field_FixedDecimal::SetFromString(Record* pRecord, const char* pOrigVal, size_t nLenOrig) const
{
	if (nLenOrig == 0)
	{
		SetNull(pRecord);
		return;
	}
	// if it goes through as a wide string 1st, this can happen.
	// I really mean to do pointer comparison here...
	if (pOrigVal != m_astrTemp.c_str())
	{
		m_astrTemp.Truncate(0);
		m_astrTemp.Append(pOrigVal, unsigned(nLenOrig));
	}
	if (m_astrTemp.c_str()[0] == '.')
		m_astrTemp = "0" + m_astrTemp;
	else if (m_astrTemp.c_str()[0] == '-' && m_astrTemp.c_str()[1] == '.')
		m_astrTemp = AString("-0") + (m_astrTemp.c_str() + 1);
	const char* pVal = m_astrTemp;
	size_t nLen = m_astrTemp.Length();

	// validate the incoming string 1st.
	const char* pEnd = pVal + nLen;
	const char* p = pVal;
	if (*p == '+' || *p == '-')
		p++;
	bool bInvalid = !IsDigit(*p);
	int nNumAfterDecimal = 0;
	if (!bInvalid)
	{
		while (p < pEnd && IsDigit(*p))
			++p;

		if (*p == '.')
		{
			++p;
			bInvalid = p == pEnd;
			while (p < pEnd && IsDigit(*p))
			{
				++nNumAfterDecimal;
				++p;
			}
		}
	}

	if (p != pEnd || bInvalid)
	{
		SetNull(pRecord);
		ReportFieldConversionError(XMSG(
			"\"@1\" is not a valid FixedDecimal. FixedDecimal values must be of the form: -nnn.nn",
			ConvertToWString(pVal)));
		return;
	}

	if (nNumAfterDecimal > this->m_nScale)
	{
		int nNewLen = m_astrTemp.Length() - (nNumAfterDecimal - this->m_nScale);
		char droppedDigit = m_astrTemp[unsigned(nNewLen)];

		if (this->m_nScale == 0)
		{
			--nNewLen;  // get rid of the .
		}

		m_astrTemp.Truncate(nNewLen);

		if (droppedDigit >= '5' && droppedDigit <= '9')
		{
			char* pRoundVal = m_astrTemp.Lock();
			int digitToRoundUp = nNewLen - 1;
			for (;;)
			{
				if (digitToRoundUp < 0)
				{
					m_astrTemp.Unlock();
					m_astrTemp = "1" + m_astrTemp;
					break;
				}

				switch (pRoundVal[digitToRoundUp])
				{
					case '+':
						pRoundVal[digitToRoundUp] = '1';
						m_astrTemp.Unlock();
						break;
					case '-':
						m_astrTemp.Unlock();
						assert(digitToRoundUp == 0);
						m_astrTemp = AString("-1") + (m_astrTemp.c_str() + digitToRoundUp + 1);
						break;
					case '.':
						digitToRoundUp--;
						continue;
					case '9':
						pRoundVal[digitToRoundUp] = '0';
						digitToRoundUp--;
						continue;
					case '0':
						pRoundVal[digitToRoundUp] = '1';
						m_astrTemp.Unlock();
						break;
					default:
						pRoundVal[digitToRoundUp]++;
						m_astrTemp.Unlock();
						break;
				}
				break;
			}
		}

		if (m_pGenericEngine)
		{
			AString strTemp = pOrigVal;
			strTemp.Truncate(nLenOrig > 64 ? 64 : unsigned(nLenOrig));
			if (nLenOrig > 64)
				strTemp += "...";

			ReportFieldConversionError(
				XMSG("\"@1\" has too many digits after the decimal and was truncated.", ConvertToWString(strTemp)));
		}
	}
	if (nNumAfterDecimal == 0 && this->m_nScale != 0)
		m_astrTemp += '.';

	while (nNumAfterDecimal < this->m_nScale)
	{
		m_astrTemp += '0';
		++nNumAfterDecimal;
	}

	if (m_astrTemp.Length() > this->m_nSize)
	{
		SetNull(pRecord);
		ReportFieldConversionError(
			XMSG("\"@1\" was too long to fit in this FixedDecimal", ConvertToWString(m_astrTemp)));
		return;
	}
	Field_String_GetSet<char>::SetVal(this, pRecord, GetOffset(), m_nSize, m_astrTemp, m_astrTemp.Length());
}

/*virtual*/ void Field_FixedDecimal::SetFromString(Record* pRecord, const U16unit* pVal, size_t nLen) const
{
	ConvertString(m_astrTemp, pVal, unsigned(nLen));
	SetFromString(pRecord, m_astrTemp.c_str(), m_astrTemp.Length());
}

/*virtual*/ void Field_FixedDecimal::SetFromBlob(Record* /*pRecord*/, const BlobVal& /*val*/) const
{
	throw Error(MSG_NoXL("Internal Error in Field_FixedDecimal::SetFromBlob: Not supported."));
}

Field_FixedDecimal::Field_FixedDecimal(const StringNoCase& strFieldName, int nSize, int nScale)
	: T_Field_String<E_FT_FixedDecimal, char, Field_String_GetSet<char>, false>(strFieldName, nSize, nScale)
{
}

Field_FixedDecimal::Field_FixedDecimal(const FieldSchema& fieldSchema, int nSize, int nScale)
	: T_Field_String<E_FT_FixedDecimal, char, Field_String_GetSet<char>, false>(fieldSchema, nSize, nScale)
{
}

SmartPointerRefObj<FieldBase> Field_FixedDecimal::Copy() const
{
	return CopyHelper(new Field_FixedDecimal(GetFieldName(), m_nSize, m_nScale));
}

}  // namespace SRC
