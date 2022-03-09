#include "stdafx.h"

#include "RecordLib/FieldDateTime.h"

#include "Base/DateTimeValidate.h"

#if !defined(SRCLIB_REPLACEMENT) && !defined(E2)
	#pragma warning(push)
	#pragma warning(disable : 4244)
	#include <boost/date_time/posix_time/posix_time.hpp>
	#pragma warning(pop)
#endif

namespace SRC {
/*virtual*/ void Field_DateTime_Base::SetFromBool(Record* /*pRecord*/, bool /*bVal*/) const
{
	throw Error(XMSG("Date/Time fields do not support Conversion from Bool"));
}

/*virtual*/ void Field_DateTime_Base::SetFromInt32(Record* /*pRecord*/, int /*nVal*/) const
{
	throw Error(XMSG("Date/Time fields do not support Conversion from Int32"));
}

/*virtual*/ void Field_DateTime_Base::SetFromInt64(Record* /*pRecord*/, int64_t /*nVal*/) const
{
	throw Error(XMSG("Date/Time fields do not support Conversion from Int64"));
}

/*virtual*/ void Field_DateTime_Base::SetFromDouble(Record* pRecord, double dVal) const
{
#if !defined(SRCLIB_REPLACEMENT) && !defined(E2)
	auto WriteNumber4 = [](char* pDestLSB, int nValue) {
		*pDestLSB = nValue % 10 + '0';
		--pDestLSB;
		nValue /= 10;

		*pDestLSB = nValue % 10 + '0';
		--pDestLSB;
		nValue /= 10;

		*pDestLSB = nValue % 10 + '0';
		--pDestLSB;
		nValue /= 10;

		*pDestLSB = nValue % 10 + '0';
	};

	auto WriteNumber2 = [](char* pDestLSB, int nValue) {
		*pDestLSB = nValue % 10 + '0';
		--pDestLSB;
		nValue /= 10;

		*pDestLSB = nValue % 10 + '0';
	};

	if (dVal < 0 || dVal >= 2958466)  // double value must be between 12/30/1899 and 12/31/9999
	{
		SetNull(pRecord);
		if (m_pGenericEngine)
		{
			if (dVal < 0)
				ReportFieldConversionError(
					XMSG("\"@1\" is an invalid datetime - Earliest date supported is Dec 30, 1899 (0)", String(dVal)));
			else
				ReportFieldConversionError(XMSG(
					"\"@1\" is an invalid datetime - Latest date supported is Dec 31, 9999 (2958466)", String(dVal)));
		}
		return;
	}

	char buffer[20];
	const boost::gregorian::date d1(1899, 12, 30);                           // create date that is == 0
	boost::gregorian::date_duration dd(static_cast<int>(dVal));              // strip the time portion from double
	boost::gregorian::greg_year_month_day ymd = (d1 + dd).year_month_day();  // add our date to zero, to get YMD

	// Offsetting by a 1/2 second to avoid rounding errors below
	dVal += 0.5 / (24 * 60 * 60);

	// logic to calculate the time
	double days, hours, mins, secs;
	double hoursmins = modf(dVal, &days) * 24.0;
	double minssecs = modf(hoursmins, &hours) * 60.0;
	double secsrem = modf(minssecs, &mins) * 60.0;
	modf(secsrem, &secs);

	WriteNumber4(buffer + 3, ymd.year);
	buffer[4] = '-';
	WriteNumber2(buffer + 6, ymd.month);
	buffer[7] = '-';
	WriteNumber2(buffer + 9, ymd.day);
	buffer[10] = ' ';

	WriteNumber2(buffer + 12, static_cast<int>(hours + 0.5));
	buffer[13] = ':';

	WriteNumber2(buffer + 15, static_cast<int>(mins + 0.5));
	buffer[16] = ':';

	WriteNumber2(buffer + 18, static_cast<int>(secs + 0.5));
	buffer[19] = 0;
	SetFromString(pRecord, buffer, 19);
#else
	(void)pRecord;
	(void)dVal;
	throw Error(XMSG("Date/Time fields do not support Conversion from Double"));
#endif
}

/*virtual*/ void Field_DateTime_Base::SetFromString(Record* pRecord, const char* pVal, size_t nLen) const
{
	nLen = std::min(unsigned(nLen), m_nSize);
	T_Field_String<E_FT_String, char, Field_String_GetSet<char>, false>::SetFromString(pRecord, pVal, nLen);
	Validate(pRecord, pVal);
}

/*virtual*/ void Field_DateTime_Base::SetFromString(Record* pRecord, const U16unit* pVal, size_t nLen) const
{
	DoConvertString(m_strBuffer, pVal, int(nLen));
	SetFromString(pRecord, m_strBuffer.c_str(), m_strBuffer.Length());
}

///////////////////////////////////////////////////////////////////////////////
// class Field_Date
/*virtual*/ bool Field_Date::Validate(const AStringVal& val) const
{
	return TDateTimeValidate<char>::ValidateDate(val.pValue, val.nLength);
}

/*virtual*/ SmartPointerRefObj<FieldBase> Field_Date::Copy() const
{
	return CopyHelper(new Field_Date(GetFieldName()));
}

///////////////////////////////////////////////////////////////////////////////
// class Field_Time
/*virtual*/ bool Field_Time::Validate(const AStringVal& val) const
{
	return TDateTimeValidate<char>::ValidateTime(val.pValue, val.nLength);
}

/*virtual*/ SmartPointerRefObj<FieldBase> Field_Time::Copy() const
{
	return CopyHelper(new Field_Time(GetFieldName()));
}

/*virtual*/ void Field_Time::SetFromString(Record* pRecord, const char* pVal, size_t nLen) const
{
	if (nLen == 19 && TDateTimeValidate<char>::ValidateDate(pVal, 10))
	{
		pVal += 11;
		nLen -= 11;
	}
	Field_DateTime_Base::SetFromString(pRecord, pVal, nLen);
}

///////////////////////////////////////////////////////////////////////////////
// class Field_DateTime
/*virtual*/ bool Field_DateTime::Validate(const AStringVal& val) const
{
	return TDateTimeValidate<char>::ValidateDateTime(val.pValue, val.nLength);
}

/*virtual*/ SmartPointerRefObj<FieldBase> Field_DateTime::Copy() const
{
	return CopyHelper(new Field_DateTime(GetFieldName()));
}

/*virtual*/ void Field_DateTime::SetFromString(Record* pRecord, const char* pVal, size_t nLen) const
{
	if (nLen == 10)
	{
		AString temp = pVal;
		temp += " 00:00:00";
		Field_DateTime_Base::SetFromString(pRecord, temp.c_str(), temp.Length());
	}
	else
		Field_DateTime_Base::SetFromString(pRecord, pVal, nLen);
}

Field_DateTime_Base::Field_DateTime_Base(const StringNoCase& strFieldName, int nSize, E_FieldType ft)
	: T_Field_String<E_FT_String, char, Field_String_GetSet<char>, false>(strFieldName, nSize, -1, ft)
{
}

Field_DateTime_Base::Field_DateTime_Base(const FieldSchema& fieldSchema, int nSize, E_FieldType ft)
	: T_Field_String<E_FT_String, char, Field_String_GetSet<char>, false>(fieldSchema, nSize, -1, ft)
{
}

Field_Date::Field_Date(const StringNoCase& strFieldName)
	: Field_DateTime_Base(strFieldName, 10, E_FT_Date)
{
}

Field_Date::Field_Date(const FieldSchema& fieldSchema)
	: Field_DateTime_Base(fieldSchema, 10, E_FT_Date)
{
}

Field_Time::Field_Time(StringNoCase strFieldName)
	: Field_DateTime_Base(strFieldName, 8, E_FT_Time)
{
}

Field_Time::Field_Time(const FieldSchema& fieldSchema)
	: Field_DateTime_Base(fieldSchema, 8, E_FT_Time)
{
}

Field_DateTime::Field_DateTime(StringNoCase strFieldName)
	: Field_DateTime_Base(strFieldName, 19, E_FT_DateTime)
{
}

Field_DateTime::Field_DateTime(const FieldSchema& fieldSchema)
	: Field_DateTime_Base(fieldSchema, 19, E_FT_DateTime)
{
}

}  // namespace SRC
