#include "stdafx.h"

#include "FieldType.h"

#define ASSERTANDRETURNFT(Value, Type)                                                                                 \
	{                                                                                                                  \
		assert(SRC::String::CompareNoCase(Value, p) == 0);                                                             \
		return Type;                                                                                                   \
	}

namespace SRC {
const String& GetNameFromFieldType(E_FieldType ft)
{
	static const String s_pE_FieldTypeNames[] = {
		_U("Unknown"),  _U("Bool"),         _U("Byte"),      _U("Int16"),  _U("Int32"),
		_U("Int64"),    _U("FixedDecimal"), _U("Float"),     _U("Double"), _U("String"),
		_U("WString"),  _U("V_String"),     _U("V_WString"), _U("Date"),   _U("Time"),
		_U("DateTime"), _U("Blob"),         _U("SpatialObj")
		// GetFieldTypeFromName hardcodes this list - if change, must change there
	};

	return s_pE_FieldTypeNames[ft];
}

E_FieldType GetFieldTypeFromName(const String::TChar* p)
{
	if (!p)
		return E_FT_Unknown;

	switch (p[0])
	{
		case 0:
			// special case for an empty string to give Unknown
			return E_FT_Unknown;
		case 'B':
		case 'b':
			if (p[1] == 'o' || p[1] == 'O')
				ASSERTANDRETURNFT(_U("Bool"), E_FT_Bool)
			else if (p[1] == 'l' || p[1] == 'L')
				ASSERTANDRETURNFT(_U("Blob"), E_FT_Blob)
			ASSERTANDRETURNFT(_U("Byte"), E_FT_Byte)
		case 'I':
		case 'i':
			if (p[3] == '1')
				ASSERTANDRETURNFT(_U("Int16"), E_FT_Int16)
			else if (p[3] == '3')
				ASSERTANDRETURNFT(_U("Int32"), E_FT_Int32)
			ASSERTANDRETURNFT(_U("Int64"), E_FT_Int64)
		case 'F':
		case 'f':
			if (p[1] == 'i' || p[1] == 'I')
				ASSERTANDRETURNFT(_U("FixedDecimal"), E_FT_FixedDecimal)
			ASSERTANDRETURNFT(_U("Float"), E_FT_Float)
		case 'D':
		case 'd':
			if (p[1] == 'o' || p[1] == 'O')
				ASSERTANDRETURNFT(_U("Double"), E_FT_Double)
			else if (p[4] == 0)
				ASSERTANDRETURNFT(_U("Date"), E_FT_Date)
			ASSERTANDRETURNFT(_U("DateTime"), E_FT_DateTime)
		case 'S':
		case 's':
			if (p[1] == 't' || p[1] == 'T')
				ASSERTANDRETURNFT(_U("String"), E_FT_String)
			ASSERTANDRETURNFT(_U("SpatialObj"), E_FT_SpatialObj)
		case 'W':
		case 'w':
			ASSERTANDRETURNFT(_U("WString"), E_FT_WString)
		case 'V':
		case 'v':
			if (p[2] == 's' || p[2] == 'S')
				ASSERTANDRETURNFT(_U("V_String"), E_FT_V_String)
			ASSERTANDRETURNFT(_U("V_WString"), E_FT_V_WString)
		case 'T':
		case 't':
			ASSERTANDRETURNFT(_U("Time"), E_FT_Time)
	}
	assert(0 && "Unknown field type in GetFieldTypeFromName");
	return E_FT_Unknown;
}
}  // namespace SRC
