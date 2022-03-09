// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

#pragma once

#include "FieldBase.h"
#include "Record.h"

namespace SRC {
class RECORDLIB_EXPORT_CPP Field_Bool : public FieldBase
{
public:
	// Returns true if this character is a non-zero digit or a "T"
	static bool TestChar(U16unit c);

	template <class TNumber>
	inline static bool ConvertNumber(TNumber n)
	{
		return n != 0;
	}

private:
	TFieldVal<bool> GetVal(const RecordData* pRecord) const;
	void SetVal(Record* pRecord, bool val) const;

public:
	Field_Bool(StringNoCase strFieldName);

	Field_Bool(const FieldSchema& fieldSchema);

	SmartPointerRefObj<FieldBase> Copy() const;

	virtual TFieldVal<bool> GetAsBool(const RecordData* pRecord) const;
	virtual TFieldVal<int> GetAsInt32(const RecordData* pRecord) const;
	virtual TFieldVal<AStringVal> GetAsAString(const RecordData* pRecord) const;
	virtual TFieldVal<WStringVal> GetAsWString(const RecordData* pRecord) const;

	virtual void SetFromBool(Record* pRecord, bool bVal) const;
	virtual void SetFromInt32(Record* pRecord, int nVal) const;
	virtual void SetFromInt64(Record* pRecord, int64_t nVal) const;
	virtual void SetFromDouble(Record* pRecord, double dVal) const;
	virtual void SetFromString(Record* pRecord, const char* pVal, size_t nLen) const;
	virtual void SetFromString(Record* pRecord, const U16unit* pVal, size_t nLen) const;

	virtual bool GetNull(const RecordData* pRecord) const;
	virtual void SetNull(Record* pRecord) const;
};
}  // namespace SRC
