#include "stdafx.h"

#include "RecordLib/FieldBlob.h"

#include "RecordLib/RecordInfo.h"

#if !defined(SRCLIB_REPLACEMENT) && !defined(EXCLUDE_GEOJSON)
	#include "Base/GeoJson/GeoJson.h"
#endif

#if !defined(SRCLIB_REPLACEMENT) && !defined(E2)
	#include "Base/Blob.h"
	#include "Base/MapObj/MapObj_ShpBlob.h"
#endif

namespace SRC {
/*virtual*/ TFieldVal<int> Field_Blob::GetAsInt32(const RecordData* /*pRecord*/) const
{
	throw Error(MSG_NoXL("Internal Error in Field_Blob::GetAsInt32: Not supported."));
}

/*virtual*/ TFieldVal<AStringVal> Field_Blob::GetAsAString(const RecordData* pRecord) const
{
	TFieldVal<BlobVal> blob = GetAsBlob(pRecord);
	TFieldVal<AStringVal> ret(true, AStringVal(0u, ""));
	if (!blob.bIsNull)
	{
		if (m_ft == E_FT_SpatialObj)
		{
#if defined(SRCLIB_REPLACEMENT) || defined(EXCLUDE_GEOJSON)
			m_astrTemp = "SpatialObject";
#else
			char* achTemp = ConvertToGeoJSON(blob.value.pValue, blob.value.nLength);
			if (achTemp)
			{
				m_astrTemp = achTemp;
				free(achTemp);
			}
			else
				m_astrTemp = "[Null]";
#endif
		}
		else
		{
			// the old SDK does not have an unsigned int string::assign function
			m_astrTemp.Assign(static_cast<int64_t>(blob.value.nLength));
			m_astrTemp += " Bytes";
		}
		ret.bIsNull = false;
	}
	else
		m_astrTemp = "[Null]";
	ret.value = AStringVal(m_astrTemp.Length(), m_astrTemp.c_str());
	return ret;
}

/*virtual*/ TFieldVal<WStringVal> Field_Blob::GetAsWString(const RecordData* pRecord) const
{
	TFieldVal<BlobVal> blob = GetAsBlob(pRecord);
	TFieldVal<WStringVal> ret(true, WStringVal(0u, _U("")));
	if (!blob.bIsNull)
	{
		if (m_ft == E_FT_SpatialObj)
		{
#if defined(SRCLIB_REPLACEMENT) || defined(EXCLUDE_GEOJSON)
			m_wstrTemp = _U("SpatialObject");
#else
			char* achTemp = ConvertToGeoJSON(blob.value.pValue, blob.value.nLength);
			if (achTemp)
			{
				m_wstrTemp = ConvertToWString(achTemp);
				free(achTemp);
			}
			else
				m_wstrTemp = U16("[Null]");
#endif
		}
		else
		{
			// the old SDK does not have an unsigned int string::assign function
			m_wstrTemp.Assign(static_cast<int64_t>(blob.value.nLength));
			m_wstrTemp += U16(" Bytes");
		}
		ret.bIsNull = false;
	}
	else
		m_wstrTemp = _U("[Null]");
	ret.value = WStringVal(m_wstrTemp.Length(), m_wstrTemp.c_str());
	return ret;
}

/*virtual*/ void Field_Blob::SetFromInt32(Record* /*pRecord*/, int /*nVal*/) const
{
	throw Error(MSG_NoXL("Internal Error in Field_Blob::SetFromInt32: Not supported."));
}

/*virtual*/ void Field_Blob::SetFromInt64(Record* /*pRecord*/, int64_t /*nVal*/) const
{
	throw Error(MSG_NoXL("Internal Error in Field_Blob::SetFromInt64: Not supported."));
}

/*virtual*/ void Field_Blob::SetFromDouble(Record* /*pRecord*/, double /*dVal*/) const
{
	throw Error(MSG_NoXL("Internal Error in Field_Blob::SetFromDouble: Not supported."));
}

/*virtual*/ void Field_Blob::SetFromString(Record* /*pRecord*/, const char* /*pVal*/, size_t /*nLen*/) const
{
	throw Error(MSG_NoXL("Internal Error in Field_Blob::SetFromString: Not supported."));
}

/*virtual*/ void Field_Blob::SetFromString(Record* /*pRecord*/, const U16unit* /*pVal*/, size_t /*nLen*/) const
{
	throw Error(MSG_NoXL("Internal Error in Field_Blob::SetFromString: Not supported."));
}

/*virtual*/ TFieldVal<BlobVal> Field_Blob::GetAsBlob(const RecordData* pRecord) const
{
	TFieldVal<BlobVal> ret(false, RecordInfo::GetVarDataValue(pRecord, GetOffset()));
	ret.bIsNull = ret.value.pValue == nullptr;
	return ret;
}

/*virtual*/ TFieldVal<BlobVal> Field_Blob::GetAsSpatialBlob(const RecordData* pRecord) const
{
	//validate that it is a correct spatial object
	TFieldVal<BlobVal> ret(false, RecordInfo::GetVarDataValue(pRecord, GetOffset()));
	ret.bIsNull = ret.value.pValue == nullptr;
	if (!ret.bIsNull)
	{
#ifndef E2
		//TODO E2- do this
		BlobDataRead file(ret.value.pValue, ret.value.nLength);
		bool bValid = SHPBlob::ValidateShpBlob(file);
		if (!bValid)
			throw Error(MSG_NoXL("Internal Error in Field_Blob::GetAsSpatialBlob: Invalid SpatialBlob."));
#endif
	}
	return ret;
}

/*virtual*/ void Field_Blob::SetFromBlob(Record* pRecord, const BlobVal& val) const
{
	RecordInfo::SetVarDataValue(pRecord, GetOffset(), val.nLength, val.pValue);
}

/*virtual*/ void Field_Blob::SetFromSpatialBlob(Record* pRecord, const BlobVal& val) const
{
	RecordInfo::SetVarDataValue(pRecord, GetOffset(), val.nLength, val.pValue);
}

bool Field_Blob::GetNull(const RecordData* pRecord) const
{
	return GetAsBlob(pRecord).bIsNull;
}

/*virtual*/ void Field_Blob::SetNull(Record* pRecord) const
{
	RecordInfo::SetVarDataValue(pRecord, GetOffset(), 0, nullptr);
}

Field_Blob::Field_Blob(const StringNoCase& strFieldName, bool bIsSpatialObj)
	: FieldBase(strFieldName, bIsSpatialObj ? E_FT_SpatialObj : E_FT_Blob, 4, true, MaxFieldLength, 0)
{
}

Field_Blob::Field_Blob(const FieldSchema& fieldSchema, bool bIsSpatialObj)
	: FieldBase(
		{ fieldSchema, fieldSchema.GetFieldName(), bIsSpatialObj ? E_FT_SpatialObj : E_FT_Blob, MaxFieldLength, 0 },
		4,
		true)
{
}

SmartPointerRefObj<FieldBase> Field_Blob::Copy() const
{
	return CopyHelper(new Field_Blob(GetFieldName(), m_ft == E_FT_SpatialObj));
}

}  // namespace SRC
