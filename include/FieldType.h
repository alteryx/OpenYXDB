// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

#pragma once

#include "RecordLib/RecordLibExport.h"

#include "SrcLib_Replacement.h"

namespace SRC {

enum E_FieldType : uint8_t
{
	E_FT_Unknown,  // not really a type
	E_FT_FirstValue = E_FT_Unknown,
	E_FT_Bool,
	E_FT_Byte,  //unsigned byte
	E_FT_Int16,
	E_FT_Int32,
	E_FT_Int64,
	E_FT_FixedDecimal,
	E_FT_Float,
	E_FT_Double,
	E_FT_String,
	E_FT_WString,
	E_FT_V_String,
	E_FT_V_WString,
	E_FT_Date,      // A 10 char String in "yyyy-mm-dd" format
	E_FT_Time,      // A 8 char String in "hh:mm:ss" format
	E_FT_DateTime,  // A 19 char String in "yyyy-mm-dd hh:mm:ss" format
	E_FT_Blob,
	E_FT_SpatialObj,
	E_FT_LastValue = E_FT_SpatialObj
};

typedef bool (*IsWhat)(E_FieldType eType);

inline bool IsAny(E_FieldType)
{
	return true;
}

inline bool IsBool(E_FieldType eType)
{
	return eType == E_FT_Bool;
}

inline bool IsBoolOrInteger(E_FieldType eType)
{
	return eType == E_FT_Bool || eType == E_FT_Byte || eType == E_FT_Int16 || eType == E_FT_Int32
		   || eType == E_FT_Int64;
}

inline bool IsInteger(E_FieldType eType)
{
	return eType == E_FT_Byte || eType == E_FT_Int16 || eType == E_FT_Int32 || eType == E_FT_Int64;
}

inline bool IsFloat(E_FieldType eType)
{
	return eType == E_FT_Float || eType == E_FT_Double;
}

inline bool IsNumeric(E_FieldType eType)
{
	return eType == E_FT_Byte || eType == E_FT_Int16 || eType == E_FT_Int32 || eType == E_FT_Int64
		   || eType == E_FT_FixedDecimal || eType == E_FT_Float || eType == E_FT_Double;
}

inline bool IsString(E_FieldType eType)
{
	return eType == E_FT_String || eType == E_FT_WString || eType == E_FT_V_String || eType == E_FT_V_WString;
}

inline bool IsStringOrDate(E_FieldType eType)
{
	return eType == E_FT_String || eType == E_FT_WString || eType == E_FT_V_String || eType == E_FT_V_WString
		   || eType == E_FT_Date || eType == E_FT_Time || eType == E_FT_DateTime;
}

inline bool IsDateOrTime(E_FieldType eType)
{
	return eType == E_FT_Date || eType == E_FT_Time || eType == E_FT_DateTime;
}

inline bool IsTime(E_FieldType eType)
{
	return eType == E_FT_Time;
}

inline bool IsDate(E_FieldType eType)
{
	return eType == E_FT_Date || eType == E_FT_DateTime;
}

inline bool IsBinary(E_FieldType eType)
{
	return eType == E_FT_Blob || eType == E_FT_SpatialObj;
}

inline bool IsBlob(E_FieldType eType)
{
	return eType == E_FT_Blob;
}

inline bool IsSpatialObj(E_FieldType eType)
{
	return eType == E_FT_SpatialObj;
}

inline bool IsNotBinary(E_FieldType eType)
{
	return eType != E_FT_Blob && eType != E_FT_SpatialObj;
}

inline bool IsNotBlob(E_FieldType eType)
{
	return eType != E_FT_Blob;
}

inline bool IsNotSpatial(E_FieldType eType)
{
	return eType != E_FT_SpatialObj;
}

inline bool IsNarrowString(E_FieldType eType)
{
	return eType == E_FT_String || eType == E_FT_V_String;
}

inline bool IsWideString(E_FieldType eType)
{
	return eType == E_FT_WString || eType == E_FT_V_WString;
}

inline bool IsVariableLenString(E_FieldType eType)
{
	return eType == E_FT_V_String || eType == E_FT_V_WString;
}

RECORDLIB_EXPORT_CPP const String& GetNameFromFieldType(E_FieldType ft);
RECORDLIB_EXPORT_CPP E_FieldType GetFieldTypeFromName(const String::TChar* p);
}  // namespace SRC

namespace std {
template <>
struct hash<SRC::E_FieldType>
{
	std::size_t operator()(const SRC::E_FieldType& k) const
	{
		return k;
	}
};
}  // namespace std
