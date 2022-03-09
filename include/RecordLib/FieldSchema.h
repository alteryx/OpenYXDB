// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56549
// include guards prevent redefiniton errors while building e2

#ifndef XSRCLIB_RECORDLIB_INCLUDE_RECORDLIB_FIELDSCHEMA_H
#define XSRCLIB_RECORDLIB_INCLUDE_RECORDLIB_FIELDSCHEMA_H

#ifndef SRCLIB_REPLACEMENT
	#include <Base/Glot.h>
	#include <Base/SmartPointer.h>
	#include <Base/StringToDouble.h>
	#include "AyxData/dc/common/FieldType.h"
#else
	#include "FieldType.h"
#endif

#include "Base/MiniXmlParser.h"

#ifdef E2
	#include "e2Base/e2_SrcLib_Replacement.h"
#endif

#include <array>
#include <memory>

#include "RecordLibExport.h"

namespace e2 {
class RecordInfo;
}

namespace SRC {
class RECORDLIB_EXPORT_CPP FieldSchema : public SmartPointerRefObj_Base
{
private:
	friend class RecordInfo;
	friend class e2::RecordInfo;

protected:
	mutable int m_nPos;

	// the field name is about the only thing that can be safely changed after the field has been created
	mutable StringNoCase m_strFieldName;
	mutable String m_strSource;
	mutable String m_strDescription;

	void SetFieldName(String str) const;
	void SetFieldPosition(int nPos) const;

public:
	const unsigned m_nSize;
	const E_FieldType m_ft;
	const int16_t m_nScale;

private:
	FieldSchema FromXML(const MINIXML_NAMESPACE::MiniXmlParser::TagInfo& tagField, const String::TChar* pNamePrefix);

public:
	FieldSchema(
		const StringNoCase& strFieldName,
		E_FieldType ft,
		unsigned nSize,
		int nScale,
		String strSource = String(),
		String strDescription = String());
	FieldSchema(const MINIXML_NAMESPACE::MiniXmlParser::TagInfo& tagField, const String::TChar* pNamePrefix);
	FieldSchema(const FieldSchema& fieldSchema, const StringNoCase& strNewName);
	FieldSchema(const FieldSchema& fieldSchema, const StringNoCase& strNewName, E_FieldType ft);
	FieldSchema(const FieldSchema& fieldSchema, const StringNoCase& strNewName, E_FieldType ft, unsigned nSize);
	FieldSchema(
		const FieldSchema& fieldSchema,
		const StringNoCase& strNewName,
		E_FieldType ft,
		unsigned nSize,
		int nScale);
	FieldSchema(
		const FieldSchema& fieldSchema,
		const StringNoCase& strNewName,
		E_FieldType ft,
		unsigned nSize,
		int nScale,
		const String& strSource);

	FieldSchema(const FieldSchema& fieldSchema);

	virtual ~FieldSchema();

	String GetFieldIdAsString() const;

	StringNoCase GetFieldName() const;
	String GetSource() const;

	// the convention of the Source is the it has a scope identifier and a value
	// the value is interpretable only in that scope - the scope string cannot contain a :
	void SetSource(String strSource) const;
	void SetSource(String strScope, String strValue) const;
	String GetDescription() const;
	void SetDescription(String strDescription) const;

	// Field position inside a RecordInfo. -1 if FieldSchema not in a RecordInfo
	int GetFieldPosition() const;

	// returns true if type, name, size and scale are equal
	// if bCompareId if true - also compares id
	bool Compare(const FieldSchema& o) const;

	bool operator==(const FieldSchema& o) const;

	bool operator!=(const FieldSchema& o) const;

	// returns true if everything about the field is the same
	// other than the name can be different
	bool EqualType(const FieldSchema& o) const;
};
}  // namespace SRC

#endif /* XSRCLIB_RECORDLIB_INCLUDE_RECORDLIB_FIELDSCHEMA_H */
