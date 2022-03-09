// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56549
// include guards prevent redefiniton errors while building e2
#ifndef XSRCLIB_RECORDLIB_INCLUDE_RECORDLIB_FIELDBASE_H
#define XSRCLIB_RECORDLIB_INCLUDE_RECORDLIB_FIELDBASE_H

#include "Base/TBlobVal.h"
#include "Base/TFieldVal.h"

#include "AyxData/dc/sdk/GenericEngineBase.h"

#include "FieldSchema.h"

namespace SRC {
struct RecordData;

class Record;

////////////////////////////////////////////////////////////////////////////////////////
// class FieldBase
//
// This class is not thread safe - even when calling const functions.
// It uses internal buffers for managing the translations and 2 threads can't be using it at
// the same time
////////////////////////////////////////////////////////////////////////////////////////
class RECORDLIB_EXPORT_CPP FieldBase : public FieldSchema
{
	friend class RecordInfo;
	friend class RecordCopier;

	unsigned m_nOffset;

protected:
	FieldBase(
		const StringNoCase& strFieldName,
		E_FieldType ft,
		const int nRawSize,
		bool bIsVarLength,
		int nSize,
		int nScale);
	FieldBase(const FieldSchema& fieldSchema, const int nRawSize, bool bIsVarLength);

	FieldBase* CopyHelper(FieldBase* p) const;

	mutable AString m_astrTemp;
	mutable String m_wstrTemp;

	mutable const GenericEngineBase* m_pGenericEngine;
	mutable unsigned m_nFieldConversionErrorCount;

public:
	FieldBase(const FieldBase&) = delete;
	FieldBase& operator=(const FieldBase&) = delete;

	virtual ~FieldBase();

	virtual SmartPointerRefObj<FieldBase> Copy() const = 0;

	const unsigned m_nRawSize;
	const bool m_bIsVarLength;

	int GetOffset() const;
	virtual unsigned GetMaxBytes() const;

	bool operator==(const FieldBase& o) const;

	bool operator!=(const FieldBase& o) const;

	virtual TFieldVal<bool> GetAsBool(const RecordData* pRecord) const;
	virtual TFieldVal<int> GetAsInt32(const RecordData* pRecord) const = 0;
	virtual TFieldVal<int64_t> GetAsInt64(const RecordData* pRecord) const;
	virtual TFieldVal<double> GetAsDouble(const RecordData* pRecord) const;
	virtual TFieldVal<AStringVal> GetAsAString(const RecordData* pRecord) const = 0;
	virtual TFieldVal<WStringVal> GetAsWString(const RecordData* pRecord) const = 0;
	virtual TFieldVal<BlobVal> GetAsBlob(const RecordData* pRecord) const;
	virtual TFieldVal<BlobVal> GetAsSpatialBlob(const RecordData* pRecord) const;

	virtual void SetFromBool(Record* pRecord, bool bVal) const;
	virtual void SetFromInt32(Record* pRecord, int nVal) const = 0;
	virtual void SetFromInt64(Record* pRecord, int64_t nVal) const = 0;
	virtual void SetFromDouble(Record* pRecord, double dVal) const = 0;
	virtual void SetFromString(Record* pRecord, const char* pVal, size_t nLen) const = 0;
	virtual void SetFromString(Record* pRecord, const U16unit* pVal, size_t nLen) const = 0;
	virtual void SetFromBlob(Record* pRecord, const BlobVal& val) const;
	virtual void SetFromSpatialBlob(Record* pRecord, const BlobVal& val) const;

	// helpers for dealing with copying the NULL
	void SetFromBool(Record* pRecord, const TFieldVal<bool> val) const;
	void SetFromInt32(Record* pRecord, const TFieldVal<int>& val) const;
	void SetFromInt64(Record* pRecord, const TFieldVal<int64_t>& val) const;
	void SetFromDouble(Record* pRecord, const TFieldVal<double>& val) const;
	void SetFromString(Record* pRecord, const TFieldVal<AStringVal>& val) const;
	void SetFromString(Record* pRecord, const TFieldVal<WStringVal>& val) const;
	void SetFromString(Record* pRecord, const AString& strVal) const;
	void SetFromString(Record* pRecord, const WString& strVal) const;
	void SetFromString(Record* pRecord, const char* pVal) const;
	virtual void SetFromString(Record* pRecord, const U16unit* pVal) const;

	void SetFromBlob(Record* pRecord, const TFieldVal<BlobVal>& val) const;
	void SetFromSpatialBlob(Record* pRecord, const TFieldVal<BlobVal>& val) const;

	virtual bool GetNull(const RecordData* pRecord) const = 0;
	/// any of the other Sets should reset the null status
	virtual void SetNull(Record* pRecord) const = 0;

	const GenericEngineBase* GetGenericEngine() const;
	void ReportFieldConversionError(const String::TChar* pMessage) const;
	bool IsReportingFieldConversionErrors() const;
};
}  // namespace SRC

#endif /* XSRCLIB_RECORDLIB_INCLUDE_RECORDLIB_FIELDBASE_H */
