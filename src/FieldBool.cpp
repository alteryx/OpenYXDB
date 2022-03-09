#include "stdafx.h"

#include "RecordLib/FieldBool.h"

namespace SRC {
TFieldVal<bool> Field_Bool::GetVal(const RecordData* pRecord) const
{
	TFieldVal<bool> ret(false, false);
	char c = *(ToCharP(pRecord) + GetOffset());
	ret.bIsNull = (c & 2) != 0;  // Null
	if (!ret.bIsNull)
		ret.value = (c & 1) != 0;

	return ret;
}

TFieldVal<bool> Field_Bool::GetAsBool(const RecordData* pRecord) const
{
	return GetVal(pRecord);
}

TFieldVal<int> Field_Bool::GetAsInt32(const RecordData* pRecord) const
{
	TFieldVal<bool> val = GetVal(pRecord);
	return TFieldVal<int>(val.bIsNull, val.value);
}

TFieldVal<AStringVal> Field_Bool::GetAsAString(const RecordData* pRecord) const
{
	TFieldVal<bool> val = GetVal(pRecord);
	TFieldVal<AStringVal> ret(true, AStringVal(0u, ""));
	if (!val.bIsNull)
	{
		m_astrTemp = val.value ? "True" : "False";
		ret.value = AStringVal(m_astrTemp.Length(), m_astrTemp.c_str());
		ret.bIsNull = false;
	}
	return ret;
}

TFieldVal<WStringVal> Field_Bool::GetAsWString(const RecordData* pRecord) const
{
	TFieldVal<bool> val = GetVal(pRecord);
	TFieldVal<WStringVal> ret(true, WStringVal(0u, _U("")));
	if (!val.bIsNull)
	{
		m_wstrTemp = val.value ? _U("True") : _U("False");
		ret.value = WStringVal(m_wstrTemp.Length(), m_wstrTemp.c_str());
		ret.bIsNull = false;
	}
	return ret;
}

void Field_Bool::SetVal(Record* pRecord, bool bVal) const
{
	// the by default resets the Null flag.
	*(ToCharP(pRecord->GetRecord()) + GetOffset()) = bVal ? 1 : 0;
}

void Field_Bool::SetFromBool(Record* pRecord, bool bVal) const
{
	SetVal(pRecord, bVal);
}

void Field_Bool::SetFromInt32(Record* pRecord, int nVal) const
{
	SetVal(pRecord, nVal != 0);
}

void Field_Bool::SetFromInt64(Record* pRecord, int64_t nVal) const
{
	SetVal(pRecord, nVal != 0);
}

void Field_Bool::SetFromDouble(Record* pRecord, double dVal) const
{
	SetVal(pRecord, dVal != 0);
}

void Field_Bool::SetFromString(Record* pRecord, const char* pVal, size_t /*nLen*/) const
{
	SetVal(pRecord, TestChar(*pVal));
}

void Field_Bool::SetFromString(Record* pRecord, const U16unit* pVal, size_t /*nLen*/) const
{
	SetVal(pRecord, TestChar(*pVal));
}

bool Field_Bool::GetNull(const RecordData* pRecord) const
{
	return GetVal(pRecord).bIsNull;
}

void Field_Bool::SetNull(Record* pRecord) const
{
	*(ToCharP(pRecord->GetRecord()) + GetOffset()) = 2;
}

bool Field_Bool::TestChar(U16unit c)
{
	return (c != '0' && iswdigit(c)) || 'T' == towupper(c);
}

Field_Bool::Field_Bool(StringNoCase strFieldName)
	: FieldBase(strFieldName, E_FT_Bool, 1, false, 1, 0)
{
}

Field_Bool::Field_Bool(const FieldSchema& fieldSchema)
	: FieldBase({ fieldSchema, fieldSchema.GetFieldName(), E_FT_Bool, 1, 0 }, 1, false)
{
}

SmartPointerRefObj<FieldBase> Field_Bool::Copy() const
{
	return CopyHelper(new Field_Bool(GetFieldName()));
}

}  // namespace SRC
