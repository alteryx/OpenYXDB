// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

#pragma once

#include "FieldBase.h"
#include "FieldBool.h"
#include "RecordInfo.h"

#if !defined(SRCLIB_REPLACEMENT) && !defined(EXCLUDE_GEOJSON)
	#include "Base/GeoJson/GeoJson.h"
#endif

#include <cmath>

#ifndef __GNUG__
	#pragma warning(push)
	#pragma warning(disable : 4127)  // conditional expression is constant
#endif

namespace SRC {
template <class TChar>
class Field_String_GetSet_Buffer
{
protected:
	mutable TChar* m_pBuffer;
	mutable unsigned m_nBufferSize;

	inline void Allocate(unsigned nSize) const
	{
		// round up to the nearest power of 2.
		unsigned alloc_size = 256;
		while (alloc_size < nSize)
			alloc_size <<= 1;

		if (m_pBuffer)
			delete[] m_pBuffer;

		m_pBuffer = new TChar[nSize];
		m_nBufferSize = nSize;
	}

protected:
	inline Field_String_GetSet_Buffer()
		: m_pBuffer(NULL)
		, m_nBufferSize(0)
	{
	}
	inline ~Field_String_GetSet_Buffer()
	{
		if (m_pBuffer)
			delete[] m_pBuffer;
	}
};

template <class TChar>
class Field_String_GetSet : public Field_String_GetSet_Buffer<TChar>
{
public:
	inline static TFieldVal<char> GetVal1stChar(const RecordData* pRecord, int nOffset, int nFieldLen)
	{
		TFieldVal<char> ret(false, 0);

		ret.bIsNull = 0 != *(ToCharP(pRecord) + nOffset + nFieldLen * sizeof(TChar));
		if (!ret.bIsNull)
			ret.value = *(ToCharP(pRecord) + nOffset);

		return ret;
	}

	inline TFieldVal<TBlobVal<TChar>> GetVal(const RecordData* pRecord, unsigned nOffset, unsigned nFieldLen) const
	{
		TFieldVal<TBlobVal<TChar>> ret;
		ret.bIsNull = 0 != *(ToCharP(pRecord) + nOffset + nFieldLen * sizeof(TChar));
		if (!ret.bIsNull)
		{
			if (sizeof(TChar) == 2)
			{
				if (this->m_nBufferSize <= nFieldLen)
					this->Allocate(nFieldLen + 1);
				memcpy(this->m_pBuffer, ToCharP(pRecord) + nOffset, nFieldLen * sizeof(TChar));
				this->m_pBuffer[nFieldLen] = 0;
				ret.value.pValue = this->m_pBuffer;
			}
			else
			{
				// we don't need to copy the buffer, it should be NULL terminated already.
				// since the NULL flag is 0 if it is NOT NULL and follows the string
				ret.value.pValue = reinterpret_cast<const TChar*>(ToCharP(pRecord) + nOffset);
			}

			const TChar* p;
			for (p = ret.value.pValue; *p; ++p)
				;
			ret.value.nLength = unsigned(p - ret.value.pValue);
		}

		return ret;
	}

	inline static void SetVal(
		const FieldBase* pField,
		Record* pRecord,
		int nOffset,
		size_t nFieldLen,
		const TChar* pVal,
		size_t nLen)
	{
		if (nLen > nFieldLen && pField->GetGenericEngine())
			pField->ReportFieldConversionError(XMSG("\"@1\" was truncated", ConvertToString(pVal)));

		char* pFieldData = ToCharP(pRecord->GetRecord()) + nOffset;
		// reset the Null flag
		// since the NULL flag is 0 if it is NOT NULL and follows the string
		// we always have a NULL terminated string
		*(pFieldData + nFieldLen * sizeof(TChar)) = 0;

		memcpy(pFieldData, pVal, std::min(nFieldLen, nLen) * sizeof(TChar));

		// the input is not always null terminated
		if (nLen < nFieldLen)
			reinterpret_cast<TChar*>(pFieldData)[nLen] = 0;
	}

	inline static bool GetNull(const RecordData* pRecord, int nOffset, int nFieldLen)
	{
		return *(ToCharP(pRecord) + nOffset + nFieldLen * sizeof(TChar)) != 0;
	}

	inline static void SetNull(Record* pRecord, int nOffset, int nFieldLen)
	{
		*(ToCharP(pRecord->GetRecord()) + nOffset + nFieldLen * sizeof(TChar)) = 1;
	}

	inline static TFieldVal<BlobVal> GetAsBlob(const RecordData* pRecord, int nOffset, int nFieldLen)
	{
		TFieldVal<BlobVal> ret(false, BlobVal(0, NULL));
		ret.bIsNull = 0 != *(ToCharP(pRecord) + nOffset + nFieldLen * sizeof(TChar));
		if (!ret.bIsNull)
		{
			ret.value.pValue = (ToCharP(pRecord) + nOffset);
			ret.value.nLength = nFieldLen * sizeof(TChar);
		}
		return ret;
	}
};

template <class TChar>
class Field_V_String_GetSet : public Field_String_GetSet_Buffer<TChar>
{
public:
	inline static TFieldVal<char> GetVal1stChar(const RecordData* pRecord, int nOffset, int /*nFieldLen*/)
	{
		BlobVal val = RecordInfo::GetVarDataValue(pRecord, nOffset);
		if (val.pValue == NULL)
			return TFieldVal<char>{ true, '\0' };
		else
			return TFieldVal<char>{ false, *static_cast<const char*>(val.pValue) };
	}
	inline TFieldVal<TBlobVal<TChar>> GetVal(const RecordData* pRecord, unsigned nOffset, unsigned nFieldLen) const
	{
		BlobVal val = RecordInfo::GetVarDataValue(pRecord, nOffset);
		TFieldVal<TBlobVal<TChar>> ret;
		ret.bIsNull = val.pValue == NULL;
		if (!ret.bIsNull)
		{
			ret.value.nLength = val.nLength / sizeof(TChar);

			assert(ret.value.nLength <= nFieldLen);
#ifndef __GNUG__
			nFieldLen;  // remove warning in release
#endif

			if (this->m_nBufferSize <= ret.value.nLength)
				this->Allocate(ret.value.nLength + 1);
			memcpy(this->m_pBuffer, static_cast<const char*>(val.pValue), val.nLength);
			this->m_pBuffer[ret.value.nLength] = 0;
			ret.value.pValue = this->m_pBuffer;
		}

		return ret;
	}

	inline static void SetVal(
		const FieldBase* pField,
		Record* pRecord,
		int nOffset,
		size_t nFieldLen,
		const TChar* pVal,
		size_t nLen)
	{
		if (nLen > nFieldLen && pField->GetGenericEngine())
		{
			if (nFieldLen > 100)
			{
				TChar buffer[101];
				memcpy(buffer, pVal, 97 * sizeof(TChar));
				buffer[97] = '.';
				buffer[98] = '.';
				buffer[99] = '.';
				buffer[100] = 0;
				pField->ReportFieldConversionError(XMSG("\"@1\" was truncated", ConvertToString(buffer)));
			}
			else
				pField->ReportFieldConversionError(XMSG("\"@1\" was truncated", ConvertToString(pVal)));
		}

		RecordInfo::SetVarDataValue(pRecord, nOffset, unsigned(std::min(nFieldLen, nLen) * sizeof(TChar)), pVal);
	}

	inline static bool GetNull(const RecordData* pRecord, int nOffset, int /*nFieldLen*/)
	{
		return RecordInfo::GetVarDataValue(pRecord, nOffset).pValue == NULL;
	}

	inline static void SetNull(Record* pRecord, int nOffset, int /*nFieldLen*/)
	{
		RecordInfo::SetVarDataValue(pRecord, nOffset, 0, NULL);
	}

	inline static TFieldVal<BlobVal> GetAsBlob(const RecordData* pRecord, int nOffset, int /*nFieldLen*/)
	{
		TFieldVal<BlobVal> ret(false, RecordInfo::GetVarDataValue(pRecord, nOffset));
		ret.bIsNull = ret.value.pValue == NULL;
		return ret;
	}
};

//////////////////////////////////////////////////////
/* helper function for converting string to double */
template <class TChar>
TFieldVal<double> ConvertStringToDoubleWithConversionErrors(
	const TFieldVal<TBlobVal<TChar>>& val,
	const FieldBase* pField,
	const char sep = '.')
{
	TFieldVal<double> ret(val.bIsNull, 0.0);
	unsigned nLen = val.value.nLength;
	if (nLen == 0)
		ret.bIsNull = true;
	if (!ret.bIsNull)
	{
		unsigned nNumCharsUsed = ConvertToDouble(val.value.pValue, ret.value, sep);
		if (0 == nNumCharsUsed || std::isnan(ret.value))
		{
			if (pField->IsReportingFieldConversionErrors())
				pField->ReportFieldConversionError(
					XMSG("@1 is not a valid number.", ConvertToString(val.value.pValue)));
			ret.bIsNull = true;
			ret.value = 0.0;
		}
		else if (pField->IsReportingFieldConversionErrors() && nNumCharsUsed != nLen)
		{
			if (val.value.pValue[nNumCharsUsed] == ',')
				pField->ReportFieldConversionError(
					XMSG("@1 stopped converting at a comma. It might be invalid.", ConvertToString(val.value.pValue)));
			else
				pField->ReportFieldConversionError(
					XMSG("@1 lost information in translation", ConvertToString(val.value.pValue)));
		}
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////
// class Field_String
template <E_FieldType ft, class TChar, class TStorage, bool bIsVarData>
class T_Field_String : public FieldBase
{
	// a temporary spot for holding a spatial object after converting from gjson
	mutable unsigned char* m_pSpatialObj;

protected:
	mutable TStorage m_storage;
	mutable Tstr<TChar> m_strBuffer;

	//returns true if it should be NULL
	template <class TCharInner>
	inline bool CheckNumberConvError(const TCharInner* pVal, size_t nLen, const TCharInner* pEnd, bool bInt64) const
	{
		if (errno == ERANGE)
		{
			if (bInt64)
				ReportFieldConversionError(XMSG("@1 does not fit in an Int64.", ConvertToWString(pVal)));
			else
				ReportFieldConversionError(XMSG("@1 does not fit in an Int32.", ConvertToWString(pVal)));
			return true;
		}

		if (pEnd != (pVal + nLen) || nLen == 0)
		{
			if (pEnd == pVal || nLen == 0)
			{
				if (nLen > 0 && m_pGenericEngine)
					ReportFieldConversionError(XMSG("@1 is not a number.", ConvertToWString(pVal)));
				return true;
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
					ReportFieldConversionError(XMSG("@1 lost information in translation", ConvertToWString(pVal)));
			}
		}
		return false;
	}
#ifdef E2
	inline void DoConvertString(UString& strBuffer, const U16unit* pVal, size_t nLen) const
	{
		ConvertString(strBuffer, pVal, int(nLen));
	}
#endif
	inline void DoConvertString(WString& strBuffer, const U16unit* pVal, size_t /*nLen*/) const
	{
		// this should not ever be called.  It is only implemented because of template stuff
		assert(false);
		strBuffer = pVal;
	}

	inline void DoConvertString(AString& strBuffer, const U16unit* pVal, size_t nLen) const
	{
		if (nLen < 0)
			nLen = unsigned(SRC::WString::strlen(pVal));

		int bConversionError = false;
		if (nLen == 0)
			strBuffer.Truncate(0);
		else
		{
			char* pRet = strBuffer.Lock(static_cast<int>(nLen));
			for (unsigned x = 0; x < nLen; ++x)
			{
				if (pVal[x] >= 256)
				{
					pRet[x] = '?';
					bConversionError = true;
				}
				else
					pRet[x] = char(pVal[x]);
			}

			assert(nLen < (size_t)std::numeric_limits<int>::max());
			strBuffer.Unlock(static_cast<int>(nLen));
		}
		if (bConversionError)
		{
			// do the conversion again, this time allowing best fit characters.
			// we wanted to generate a Conv Error if anything changed, but now we want to get the
			// best guess to the user
#ifndef __GNUG__
			char* pRet = strBuffer.Lock(static_cast<int>(nLen + 1));

			::WideCharToMultiByte(
				28591, 0, pVal, unsigned(nLen), pRet, static_cast<int>(nLen + 1), "?", &bConversionError);
			pRet[nLen] = 0;
			strBuffer.Unlock();
#endif
			if (m_pGenericEngine)
				ReportFieldConversionError(
					XMSG("\"@1\" could not be fully converted from a WString to a String.", pVal));
		}
	}

public:
	inline T_Field_String(StringNoCase strFieldName, int nFieldSize, int nScale = -1, E_FieldType nFieldType = ft)
		: FieldBase(
			strFieldName,
			nFieldType,
			bIsVarData ? 4 : nFieldSize * sizeof(TChar) + 1,
			bIsVarData,
			nFieldSize,
			nScale)
		, m_pSpatialObj(NULL)
	{
	}

	inline T_Field_String(
		const FieldSchema& fieldSchema,
		unsigned int nFieldSize,
		int nScale = -1,
		E_FieldType nFieldType = ft)
		: FieldBase(
			{ fieldSchema, fieldSchema.GetFieldName(), nFieldType, nFieldSize, nScale },
			bIsVarData ? 4 : nFieldSize * sizeof(TChar) + 1,
			bIsVarData)
		, m_pSpatialObj(NULL)
	{
	}

	virtual ~T_Field_String()
	{
		if (m_pSpatialObj)
			free(m_pSpatialObj);
	}

	virtual SmartPointerRefObj<FieldBase> Copy() const
	{
		return CopyHelper(new T_Field_String<ft, TChar, TStorage, bIsVarData>(GetFieldName(), m_nSize, m_nScale, m_ft));
	}

public:
	/*virtual*/ unsigned GetMaxBytes() const
	{
		return m_nSize * sizeof(TChar);
	}

	virtual TFieldVal<bool> GetAsBool(const RecordData* pRecord) const
	{
		TFieldVal<char> val = TStorage::GetVal1stChar(pRecord, GetOffset(), m_nSize);
		TFieldVal<bool> ret(val.bIsNull, false);
		if (!ret.bIsNull)
			ret.value = Field_Bool::TestChar(val.value);
		return ret;
	}

	virtual TFieldVal<int> GetAsInt32(const RecordData* pRecord) const
	{
		TFieldVal<TBlobVal<TChar>> val = m_storage.GetVal(pRecord, GetOffset(), m_nSize);
		TFieldVal<int> ret(val.bIsNull, 0);
		if (!ret.bIsNull)
		{
			int base = 10;
			errno = 0;
			if (sizeof(TChar) == 1)
			{
				const char* pBegin = (const char*)val.value.pValue;
				char* pEnd;
				if (pBegin[0] == '0' && (pBegin[1] == 'x' || pBegin[1] == 'X'))
					base = 16;
				ret.value = strtol(pBegin, &pEnd, base);
				ret.bIsNull = CheckNumberConvError(pBegin, val.value.nLength, pEnd, false);
			}
			else
			{
				const U16unit* pBegin = (const U16unit*)val.value.pValue;
				U16unit* pEnd;
				if (pBegin[0] == '0' && (pBegin[1] == 'x' || pBegin[1] == 'X'))
					base = 16;
				ret.value = StringHelper::sh_strtoi(pBegin, &pEnd, base);
				ret.bIsNull = CheckNumberConvError(pBegin, val.value.nLength, pEnd, false);
			}
		}
		return ret;
	}

	virtual TFieldVal<int64_t> GetAsInt64(const RecordData* pRecord) const
	{
		TFieldVal<TBlobVal<TChar>> val(m_storage.GetVal(pRecord, GetOffset(), m_nSize));
		TFieldVal<int64_t> ret(val.bIsNull, 0);
		if (!ret.bIsNull)
		{
			// you can have hex, but you don't get to use leading spaces or sign
			int base = 10;
			errno = 0;
			if (sizeof(TChar) == 1)
			{
				const char* pBegin = (const char*)val.value.pValue;
				char* pEnd;
				if (pBegin[0] == '0' && (pBegin[1] == 'x' || pBegin[1] == 'X'))
					base = 16;
				ret.value = StringHelper::sh_strtoi64(pBegin, &pEnd, base);
				ret.bIsNull = CheckNumberConvError(pBegin, val.value.nLength, pEnd, true);
			}
			else
			{
				const U16unit* pBegin = (const U16unit*)val.value.pValue;
				U16unit* pEnd;
				if (pBegin[0] == '0' && (pBegin[1] == 'x' || pBegin[1] == 'X'))
					base = 16;
				ret.value = StringHelper::sh_strtoi64(pBegin, &pEnd, base);
				ret.bIsNull = CheckNumberConvError(pBegin, val.value.nLength, pEnd, true);
			}
		}
		return ret;
	}

	virtual TFieldVal<double> GetAsDouble(const RecordData* pRecord) const
	{
		TFieldVal<TBlobVal<TChar>> val(m_storage.GetVal(pRecord, GetOffset(), m_nSize));
		return ConvertStringToDoubleWithConversionErrors(val, this);
	}

	inline TFieldVal<AStringVal> GetAsAString(const TFieldVal<TBlobVal<char>>& val) const
	{
		return reinterpret_cast<const TFieldVal<AStringVal>&>(val);
	}

	inline TFieldVal<AStringVal> GetAsAString(const TFieldVal<TBlobVal<U16unit>>& val) const
	{
		TFieldVal<AStringVal> ret;

		ret.bIsNull = val.bIsNull;
		DoConvertString(m_astrTemp, (const U16unit*)val.value.pValue, val.value.nLength);
		ret.value = AStringVal(m_astrTemp.Length(), m_astrTemp.c_str());
		return ret;
	}

	inline TFieldVal<AStringVal> GetAsAString(const TFieldVal<WStringVal>& val) const
	{
		// TODO E2 - should this be here
		TFieldVal<AStringVal> ret;

		ret.bIsNull = val.bIsNull;
		DoConvertString(m_astrTemp, (const U16unit*)val.value.pValue, val.value.nLength);
		ret.value = AStringVal(m_astrTemp.Length(), m_astrTemp.c_str());
		return ret;
	}

	virtual TFieldVal<AStringVal> GetAsAString(const RecordData* pRecord) const
	{
		auto temp = m_storage.GetVal(pRecord, GetOffset(), m_nSize);
		return GetAsAString(temp);
	}
#ifdef E2
	inline TFieldVal<WStringVal> GetAsWString(const TFieldVal<TBlobVal<U16unit>>& val) const
	{
		TFieldVal<WStringVal> ret;

		ret.bIsNull = val.bIsNull;
		ConvertString(m_wstrTemp, val.value.pValue, val.value.nLength);
		ret.value = WStringVal(m_wstrTemp.Length(), m_wstrTemp.c_str());
		return ret;
	}
#else
	inline TFieldVal<WStringVal> GetAsWString(const TFieldVal<TBlobVal<U16unit>>& val) const
	{
		return reinterpret_cast<const TFieldVal<WStringVal>&>(val);
	}
#endif
	inline TFieldVal<WStringVal> GetAsWString(const TFieldVal<TBlobVal<char>>& val) const
	{
		TFieldVal<WStringVal> ret;

		ret.bIsNull = val.bIsNull;
		ConvertString(m_wstrTemp, val.value.pValue, static_cast<int>(val.value.nLength));
		ret.value = WStringVal(m_wstrTemp.Length(), m_wstrTemp.c_str());
		return ret;
	}

	virtual TFieldVal<WStringVal> GetAsWString(const RecordData* pRecord) const
	{
		return GetAsWString(m_storage.GetVal(pRecord, GetOffset(), m_nSize));
	}

	virtual TFieldVal<BlobVal> GetAsBlob(const RecordData* pRecord) const
	{
		return TStorage::GetAsBlob(pRecord, GetOffset(), m_nSize);
	}

	virtual TFieldVal<BlobVal> GetAsSpatialBlob(const RecordData* pRecord) const
	{
		TFieldVal<BlobVal> ret;
#if !defined(SRCLIB_REPLACEMENT) && !defined(EXCLUDE_GEOJSON)
		unsigned int len = 0;
#endif
		TFieldVal<AStringVal> val = GetAsAString(m_storage.GetVal(pRecord, GetOffset(), m_nSize));
		ret.bIsNull = val.bIsNull || val.value.nLength == 0;

		if (!ret.bIsNull)
		{
			try
			{
				if (m_pSpatialObj)
				{
					free(m_pSpatialObj);
					m_pSpatialObj = NULL;
				}
#if !defined(SRCLIB_REPLACEMENT) && !defined(EXCLUDE_GEOJSON)
				ConvertFromGeoJSON(val.value.pValue, &m_pSpatialObj, &len);
				if (len > 0)
				{
					ret.value.pValue = reinterpret_cast<unsigned char*>(m_pSpatialObj);
					ret.value.nLength = len;
					return ret;
				}
				if (IsReportingFieldConversionErrors())
				{
					int nPrintableSize = 64;

					TFieldVal<WStringVal> valPrintable = GetAsWString(m_storage.GetVal(pRecord, GetOffset(), m_nSize));
					String strToPrint = WString(valPrintable.value.pValue);

					ReportFieldConversionError(XMSG(
						"No conversion from @1 to SpatialObj",
						strToPrint.Length() > 64 ? strToPrint.Truncate(nPrintableSize) + U16("...") : strToPrint));
				}

#else
				ret.bIsNull = true;
				return ret;
#endif
			}
			catch (...)
			{
				ret = TStorage::GetAsBlob(pRecord, GetOffset(), m_nSize);
			}
		}
		return ret;
	}

	virtual void SetFromInt32(Record* pRecord, int nVal) const
	{
#ifdef E2
		UNUSED(pRecord);
		UNUSED(nVal);
		throw Error(MSG_NoXL("Internal Error in SetFromInt32: E2 Conversions not allowed"));
#else
		m_strBuffer.Assign(nVal);
		TStorage::SetVal(this, pRecord, GetOffset(), m_nSize, m_strBuffer, m_strBuffer.Length());
#endif
	}

	virtual void SetFromInt64(Record* pRecord, int64_t nVal) const
	{
#ifdef E2
		UNUSED(pRecord);
		UNUSED(nVal);
		throw Error(MSG_NoXL("Internal Error in SetFromInt64: E2 Conversions not allowed"));
#else
		m_strBuffer.Assign(nVal);
		TStorage::SetVal(this, pRecord, GetOffset(), m_nSize, m_strBuffer, m_strBuffer.Length());
#endif
	}

	virtual void SetFromDouble(Record* pRecord, double dVal) const
	{
#ifdef E2
		UNUSED(pRecord);
		UNUSED(dVal);
		throw Error(MSG_NoXL("Internal Error in SetFromDouble: E2 Conversions not allowed"));
#else
		m_strBuffer.Assign(dVal);
		TStorage::SetVal(this, pRecord, GetOffset(), m_nSize, m_strBuffer, m_strBuffer.Length());
#endif
	}

	virtual void SetFromString(Record* pRecord, const char* pVal, size_t nLen) const
	{
		if (sizeof(TChar) == 1)
			TStorage::SetVal(this, pRecord, GetOffset(), m_nSize, reinterpret_cast<const TChar*>(pVal), nLen);
		else
		{
#ifdef E2
			throw Error(MSG_NoXL("Internal Error in SetFromString (a): E2 Conversions not allowed"));
#else
			ConvertString(m_strBuffer, pVal, unsigned(nLen));
			TStorage::SetVal(this, pRecord, GetOffset(), m_nSize, m_strBuffer, nLen);
#endif
		}
	}

	virtual void SetFromString(Record* pRecord, const U16unit* pVal, size_t nLen) const
	{
		if (sizeof(TChar) == 2 && sizeof(U16unit) == 2)
			TStorage::SetVal(this, pRecord, GetOffset(), m_nSize, reinterpret_cast<const TChar*>(pVal), nLen);
		else
		{
#ifdef E2
			throw Error(MSG_NoXL("Internal Error in SetFromString (w): E2 Conversions not allowed"));
#else
			DoConvertString(m_strBuffer, pVal, unsigned(nLen));
			TStorage::SetVal(this, pRecord, GetOffset(), m_nSize, m_strBuffer, nLen);
#endif
		}
	}

	virtual void SetFromBlob(Record* pRecord, const BlobVal& val) const
	{
		assert(val.nLength % sizeof(TChar) == 0);
		TStorage::SetVal(
			this, pRecord, GetOffset(), m_nSize, static_cast<const TChar*>(val.pValue), val.nLength / sizeof(TChar));
	}

	virtual bool GetNull(const RecordData* pRecord) const
	{
		return TStorage::GetNull(pRecord, GetOffset(), m_nSize);
	}

	virtual void SetNull(Record* pRecord) const
	{
		TStorage::SetNull(pRecord, GetOffset(), m_nSize);
	}

	virtual void SetFromSpatialBlob(Record* pRecord, const BlobVal& val) const
	{
#if defined(SRCLIB_REPLACEMENT) || defined(EXCLUDE_GEOJSON)
		(void)val;  // to get past compile warnings as errors
		SetNull(pRecord);
#else
		char* achTemp = NULL;
		if (val.pValue)
			achTemp = ConvertToGeoJSON(val.pValue, val.nLength);

		if (achTemp)
		{
			SetFromString(pRecord, achTemp, strlen(achTemp));
			free(achTemp);
		}
		else
			SetNull(pRecord);
#endif
	}
};

typedef T_Field_String<E_FT_String, char, Field_String_GetSet<char>, false> Field_String;
typedef T_Field_String<E_FT_V_String, char, Field_V_String_GetSet<char>, true> Field_V_String;
#ifdef E2
typedef T_Field_String<E_FT_WString, U16unit, Field_String_GetSet<U16unit>, false> Field_WString;
typedef T_Field_String<E_FT_V_WString, U16unit, Field_V_String_GetSet<U16unit>, true> Field_V_WString;
#else
typedef T_Field_String<E_FT_WString, U16unit, Field_String_GetSet<U16unit>, false> Field_WString;
typedef T_Field_String<E_FT_V_WString, U16unit, Field_V_String_GetSet<U16unit>, true> Field_V_WString;
#endif
}  // namespace SRC
#ifndef __GNUG__
	#pragma warning(pop)
#endif
