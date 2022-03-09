#pragma once
#include <memory>

#include <gtest\gtest.h>

#include "Base/GeoJson/GeoJson.h"
#include "Base/SRC_string.h"
#include "Base/Src_Error.h"
#include "RecordLib/RecordInfo.h"

namespace SRC {
//Creates types in Alteryx's default order starting with Bool (unknown is ignored).  Each
//is set with its type cast to a value.  By default, the spatial field is generated with an invalid
//blob (a fixed sequence of 4 bytes), but a valid spatial object can be created if required.
struct RecordCreator
{
	static inline std::tuple<std::unique_ptr<SRC::RecordInfo>, SRC::SmartPointerRefObj<SRC::Record>>
	CreateCompleteRecord(bool validSpatialField = false)
	{
		std::unique_ptr<SRC::RecordInfo> recordInfo(new SRC::RecordInfo());

		for (unsigned int type = SRC::E_FT_Bool; type <= SRC::E_FT_SpatialObj; ++type)
		{
			const SRC::E_FieldType castType(static_cast<SRC::E_FieldType>(type));
			const wchar_t* const name = SRC::GetNameFromFieldType(castType);
			const SRC::String fieldXml(
				recordInfo->CreateFieldXml(L"field_" + SRC::ConvertToWString(name).LowerCase(), castType, 10u));
			recordInfo->AddField(fieldXml);
		}

		//Most types can be set with their enum value cast to the correct type.
		SRC::SmartPointerRefObj<SRC::Record> record = recordInfo->CreateRecord();
		for (unsigned int type = SRC::E_FT_Bool; type <= SRC::E_FT_V_WString; ++type)
		{
			const SRC::FieldBase* const field = recordInfo->operator[](type - SRC::E_FT_Bool);
			field->SetFromInt32(record.Get(), type);
		}

		//Date/time and blobs must added separately as explicit strings
		const SRC::FieldBase* field = recordInfo->operator[](SRC::E_FT_Date - SRC::E_FT_Bool);
		field->SetFromString(record.Get(), L"1934-12-12");
		field = recordInfo->operator[](SRC::E_FT_Time - SRC::E_FT_Bool);
		field->SetFromString(record.Get(), L"12:34:56");
		field = recordInfo->operator[](SRC::E_FT_DateTime - SRC::E_FT_Bool);
		field->SetFromString(record.Get(), L"1934-12-12 12:34:56");
		field = recordInfo->operator[](SRC::E_FT_Blob - SRC::E_FT_Bool);
		field->SetFromBlob(record.Get(), SRC::BlobVal(4u, (void*)"1234"));
		field = recordInfo->operator[](SRC::E_FT_SpatialObj - SRC::E_FT_Bool);

		if (validSpatialField)
		{
			const AString geoJSON = "{\"type\":\"Feature\",\"geometry\":"
									"{\"type\":\"Point\",\"coordinates\":[0.0,10.0]}"
									"}";
			unsigned char* spatialObject(nullptr);
			unsigned int length(0u);
			ConvertFromGeoJSON(geoJSON, &spatialObject, &length);
			const SRC::BlobVal blob(length, spatialObject);
			field->SetFromSpatialBlob(record.Get(), blob);
		}
		else
		{
			field->SetFromSpatialBlob(record.Get(), SRC::BlobVal(4u, (void*)"5678"));
		}
		return std::make_tuple(std::move(recordInfo), std::move(record));
	}
};
}  // namespace SRC
