// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

#pragma once

#include "FieldString.h"

namespace SRC {
///////////////////////////////////////////////////////////////////////////////
// class Date/Time
class RECORDLIB_EXPORT_CPP Field_DateTime_Base
	: public T_Field_String<E_FT_String, char, Field_String_GetSet<char>, false>
{
	mutable AString m_strBuffer;
	template <class TChar>
	inline void Validate(Record* pRecord, const TChar* pVal) const
	{
		if (!Validate(GetAsAString(pRecord->GetRecord()).value))
		{
			SetNull(pRecord);
			if (GetGenericEngine())
				ReportFieldConversionError(XMSG(
					"\"@1\" is not a valid @2", ConvertToString(pVal), ConvertToString(GetNameFromFieldType(m_ft))));
		}
	}

protected:
	Field_DateTime_Base(const StringNoCase& strFieldName, int nSize, E_FieldType ft);

	Field_DateTime_Base(const FieldSchema& fieldSchema, int nSize, E_FieldType ft);

	virtual bool Validate(const AStringVal& val) const = 0;

public:
	virtual void SetFromBool(Record* pRecord, bool bVal) const;
	virtual void SetFromInt32(Record* pRecord, int nVal) const;
	virtual void SetFromInt64(Record* pRecord, int64_t nVal) const;
	virtual void SetFromDouble(Record* pRecord, double dVal) const;
	virtual void SetFromString(Record* pRecord, const char* pVal, size_t nLen) const;
	virtual void SetFromString(Record* pRecord, const U16unit* pVal, size_t nLen) const;
};

class RECORDLIB_EXPORT_CPP Field_Date : public Field_DateTime_Base
{
protected:
	virtual bool Validate(const AStringVal& val) const override;

public:
	Field_Date(const StringNoCase& strFieldName);
	Field_Date(const FieldSchema& fieldSchema);
	virtual SmartPointerRefObj<FieldBase> Copy() const;
};

class RECORDLIB_EXPORT_CPP Field_Time : public Field_DateTime_Base
{
protected:
	virtual bool Validate(const AStringVal& val) const override;

public:
	Field_Time(StringNoCase strFieldName);
	Field_Time(const FieldSchema& fieldSchema);
	virtual SmartPointerRefObj<FieldBase> Copy() const;
	virtual void SetFromString(Record* pRecord, const char* pVal, size_t nLen) const;
};

class RECORDLIB_EXPORT_CPP Field_DateTime : public Field_DateTime_Base
{
protected:
	virtual bool Validate(const AStringVal& val) const override;

public:
	Field_DateTime(StringNoCase strFieldName);
	Field_DateTime(const FieldSchema& fieldSchema);

	virtual SmartPointerRefObj<FieldBase> Copy() const;
	virtual void SetFromString(Record* pRecord, const char* pVal, size_t nLen) const;
};
}  // namespace SRC
