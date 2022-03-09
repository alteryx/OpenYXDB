// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

#pragma once

#include "FieldString.h"

namespace SRC {
class RECORDLIB_EXPORT_CPP Field_FixedDecimal
	: public T_Field_String<E_FT_FixedDecimal, char, Field_String_GetSet<char>, false>
{
public:
	Field_FixedDecimal(const StringNoCase& strFieldName, int nSize, int nScale);

	Field_FixedDecimal(const FieldSchema& fieldSchema, int nSize, int nScale);

	virtual SmartPointerRefObj<FieldBase> Copy() const;

	virtual TFieldVal<bool> GetAsBool(const RecordData* pRecord) const;

	// GetAsInt32, GetAsInt64, GetAsDouble etc... are handled by the String (base) class

	virtual void SetFromBool(Record* pRecord, bool bVal) const;
	virtual void SetFromInt32(Record* pRecord, int nVal) const;
	virtual void SetFromInt64(Record* pRecord, int64_t nVal) const;
	virtual void SetFromDouble(Record* pRecord, double dVal) const;
	virtual void SetFromString(Record* pRecord, const char* pVal, size_t nLen) const;
	virtual void SetFromString(Record* pRecord, const U16unit* pVal, size_t nLen) const;
	virtual void SetFromBlob(Record* pRecord, const BlobVal& val) const;
};
}  // namespace SRC
