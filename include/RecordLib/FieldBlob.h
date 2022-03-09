// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

#pragma once

#include "FieldBase.h"
#include "Record.h"

namespace SRC {
class RECORDLIB_EXPORT_CPP Field_Blob : public FieldBase
{
public:
	Field_Blob(const StringNoCase& strFieldName, bool bIsSpatialObj);

	Field_Blob(const FieldSchema& fieldSchema, bool bIsSpatialObj);

	virtual SmartPointerRefObj<FieldBase> Copy() const;

	// unsupported accessors
	virtual TFieldVal<int> GetAsInt32(const RecordData* pRecord) const;
	virtual TFieldVal<AStringVal> GetAsAString(const RecordData* pRecord) const;
	virtual TFieldVal<WStringVal> GetAsWString(const RecordData* pRecord) const;
	virtual void SetFromInt32(Record* pRecord, int nVal) const;
	virtual void SetFromInt64(Record* pRecord, int64_t nVal) const;
	virtual void SetFromDouble(Record* pRecord, double dVal) const;
	virtual void SetFromString(Record* pRecord, const char* pVal, size_t nLen) const;
	virtual void SetFromString(Record* pRecord, const U16unit* pVal, size_t nLen) const;

	// the only actually supported interfaces
	virtual TFieldVal<BlobVal> GetAsBlob(const RecordData* pRecord) const;
	virtual TFieldVal<BlobVal> GetAsSpatialBlob(const RecordData* pRecord) const;
	virtual void SetFromBlob(Record* pRecord, const BlobVal& val) const;
	virtual void SetFromSpatialBlob(Record* pRecord, const BlobVal& val) const;
	virtual bool GetNull(const RecordData* pRecord) const;
	virtual void SetNull(Record* pRecord) const;
};
}  // namespace SRC
