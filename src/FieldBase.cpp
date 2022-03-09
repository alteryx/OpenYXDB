#include "stdafx.h"

#include "RecordLib/FieldBase.h"

#include "RecordLib/FieldTypes.h"

namespace SRC {
FieldBase::FieldBase(
	const StringNoCase& strFieldName,
	E_FieldType ft,
	const int nRawSize,
	bool bIsVarLength,
	int nSize,
	int nScale)
	: FieldSchema(strFieldName, ft, nSize, nScale)
	, m_nOffset(0)
	, m_pGenericEngine(nullptr)
	, m_nFieldConversionErrorCount(0)
	, m_nRawSize(nRawSize)
	, m_bIsVarLength(bIsVarLength)
{
}

FieldBase::FieldBase(const FieldSchema& fieldSchema, const int nRawSize, bool bIsVarLength)
	: FieldSchema(fieldSchema)
	, m_nOffset(0)
	, m_pGenericEngine(nullptr)
	, m_nFieldConversionErrorCount(0)
	, m_nRawSize(nRawSize)
	, m_bIsVarLength(bIsVarLength)
{
}

FieldBase::~FieldBase()
{
}

FieldBase* FieldBase::CopyHelper(FieldBase* p) const
{
	p->m_nOffset = m_nOffset;
	p->SetSource(m_strSource);
	p->SetDescription(m_strDescription);
	return p;
}

void FieldBase::ReportFieldConversionError(const String::TChar* pMessage) const
{
	// you don't need to call this if m_pGenericEngine has already been cleared because
	// we've printed too many conversion errors. You can tell by calling
	// IsReportingFieldConversionErrors(), and spare the expense of constructing a
	// nice message string.
	if (m_pGenericEngine)
	{
		m_pGenericEngine->OutputMessage(
			GenericEngineBase::MT_FieldConversionError, MSG_NoXL("@1: @2", this->m_strFieldName, pMessage));

		m_nFieldConversionErrorCount++;

		if (m_nFieldConversionErrorCount == m_pGenericEngine->GetFieldConversionErrorLimit())
		{
			m_pGenericEngine->OutputMessage(
				GenericEngineBase::MT_FieldConversionLimitReached,
				XMSG("@1: Field Conversion Error Limit Reached", m_strFieldName));
			m_pGenericEngine = nullptr;
		}
	}
}

/*virtual*/ unsigned FieldBase::GetMaxBytes() const
{
	return m_nSize;
}

TFieldVal<bool> FieldBase::GetAsBool(const RecordData* pRecord) const
{
	TFieldVal<bool> ret(false, false);
	switch (m_ft)
	{
		case E_FT_String:
		case E_FT_V_String:
		{
			TFieldVal<AStringVal> val = GetAsAString(pRecord);
			if (val.bIsNull)
				ret.value = Field_Bool::TestChar(val.value.pValue[0]);
			break;
		}
		case E_FT_WString:
		case E_FT_V_WString:
		{
			TFieldVal<WStringVal> val = GetAsWString(pRecord);
			if (val.bIsNull)
				ret.value = Field_Bool::TestChar(val.value.pValue[0]);
			break;
		}
		default:
		{
			TFieldVal<int> val = GetAsInt32(pRecord);
			ret.bIsNull = val.bIsNull;
			ret.value = val.value != 0;
			break;
		}
	}
	return ret;
}

TFieldVal<int64_t> FieldBase::GetAsInt64(const RecordData* pRecord) const
{
	TFieldVal<int> val = GetAsInt32(pRecord);
	return TFieldVal<int64_t>(val.bIsNull, static_cast<int64_t>(val.value));
}

TFieldVal<double> FieldBase::GetAsDouble(const RecordData* pRecord) const
{
	TFieldVal<int> val = GetAsInt32(pRecord);
	return TFieldVal<double>(val.bIsNull, static_cast<double>(val.value));
}

TFieldVal<BlobVal> FieldBase::GetAsBlob(const RecordData* /*pRecord*/) const
{
	throw Error(XMSG("SetFromBlob: Field type @1 doesn't support Blobs.", GetNameFromFieldType(m_ft)));
}

void FieldBase::SetFromBool(Record* pRecord, bool bVal) const
{
	SetFromInt32(pRecord, bVal ? 1 : 0);
}

void FieldBase::SetFromBlob(Record* /*pRecord*/, const BlobVal& /*val*/) const
{
	throw Error(XMSG("SetFromBlob: Field type @1 doesn't support Blobs.", GetNameFromFieldType(m_ft)));
}

TFieldVal<BlobVal> FieldBase::GetAsSpatialBlob(const RecordData* /*pRecord */) const
{
	throw Error(XMSG("GetAsSpatialBlob: Field type @1 doesn't support Spatial Objects.", GetNameFromFieldType(m_ft)));
}

void FieldBase::SetFromSpatialBlob(Record* /*pRecord*/, const BlobVal& /*val*/) const
{
	throw Error(XMSG("SetFromSpatialBlob: Field type @1 doesn't support Spatial Objects.", GetNameFromFieldType(m_ft)));
}

void FieldBase::SetFromBool(Record* pRecord, const TFieldVal<bool> val) const
{
	if (val.bIsNull)
		SetNull(pRecord);
	else
		SetFromBool(pRecord, val.value);
}

void FieldBase::SetFromInt32(Record* pRecord, const TFieldVal<int>& val) const
{
	if (val.bIsNull)
		SetNull(pRecord);
	else
		SetFromInt32(pRecord, val.value);
}

void FieldBase::SetFromInt64(Record* pRecord, const TFieldVal<int64_t>& val) const
{
	if (val.bIsNull)
		SetNull(pRecord);
	else
		SetFromInt64(pRecord, val.value);
}

void FieldBase::SetFromDouble(Record* pRecord, const TFieldVal<double>& val) const
{
	if (val.bIsNull)
		SetNull(pRecord);
	else
		SetFromDouble(pRecord, val.value);
}

void FieldBase::SetFromString(Record* pRecord, const TFieldVal<AStringVal>& val) const
{
	if (val.bIsNull)
		SetNull(pRecord);
	else
		SetFromString(pRecord, val.value.pValue, val.value.nLength);
}

void FieldBase::SetFromString(Record* pRecord, const TFieldVal<WStringVal>& val) const
{
	// TODO - E2 needs to convert to U16unit
	if (val.bIsNull)
		SetNull(pRecord);
	else
		SetFromString(pRecord, val.value.pValue, val.value.nLength);
}

void FieldBase::SetFromString(Record* pRecord, const U16unit* pVal) const
{
	SetFromString(pRecord, pVal, SRC::WString::strlen(pVal));
}

void FieldBase::SetFromBlob(Record* pRecord, const TFieldVal<BlobVal>& val) const
{
	if (val.bIsNull)
		SetNull(pRecord);
	else
		SetFromBlob(pRecord, val.value);
}

void FieldBase::SetFromSpatialBlob(Record* pRecord, const TFieldVal<BlobVal>& val) const
{
	if (val.bIsNull)
		SetNull(pRecord);
	else
		SetFromSpatialBlob(pRecord, val.value);
}

int FieldBase::GetOffset() const
{
	return m_nOffset;
}

bool FieldBase::operator==(const FieldBase& o) const
{
	return FieldSchema::operator==(o) && m_nRawSize == o.m_nRawSize && m_bIsVarLength == o.m_bIsVarLength;
}

bool FieldBase::operator!=(const FieldBase& o) const
{
	return !(*this == o);
}

void FieldBase::SetFromString(Record* pRecord, const AString& strVal) const
{
	SetFromString(pRecord, strVal.c_str(), strVal.Length());
}

void FieldBase::SetFromString(Record* pRecord, const WString& strVal) const
{
	SetFromString(pRecord, strVal.c_str(), strVal.Length());
}

void FieldBase::SetFromString(Record* pRecord, const char* pVal) const
{
	SetFromString(pRecord, pVal, strlen(pVal));
}

const GenericEngineBase* FieldBase::GetGenericEngine() const
{
	return m_pGenericEngine;
}

bool FieldBase::IsReportingFieldConversionErrors() const
{
	return m_pGenericEngine != nullptr;
}

// explicitly instantiate all the template types
#ifndef __GNUG__
template Field_Byte;
template Field_Int16;
template Field_Int32;
template Field_Int64;
template Field_String;
template Field_WString;
template Field_V_String;
template Field_V_WString;
#endif
}  // namespace SRC
