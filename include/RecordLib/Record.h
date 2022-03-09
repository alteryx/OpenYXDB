// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56549
// include guards prevent redefiniton errors while building e2
#ifndef XSRCLIB_RECORDLIB_INCLUDE_RECORDLIB_RECORD_H
#define XSRCLIB_RECORDLIB_INCLUDE_RECORDLIB_RECORD_H

#include "RecordLibExport.h"

#ifndef SRCLIB_REPLACEMENT
	#include <Base/SmartPointer.h>
#endif  // !SRCLIB_REPLACEMENT

#include "RecordData.h"

#ifdef E2
	#include "e2Base/e2_SrcLib_Replacement.h"
#endif

namespace SRC {
struct RecordData;

class Record : public SmartPointerRefObj_Base
{
	friend class RecordInfo;
	friend class RecordCopier;
	// m_pRecord contains:
	// The fixed data
	// and if there is potentially Var Data
	//	int varDataTotalLen
	// and for each varData
	//	int varDataLen
	//		varData
	mutable void* m_pRecord = nullptr;

	unsigned m_nFixedRecordSize = 0;
	bool m_bContainsVarData = false;

	unsigned m_nCurrentVarDataSize = 0;
	unsigned m_nCurrentBufferSize = 0;

	mutable bool m_bVarDataLenUnset = false;

	E1ONLY_RECORDLIB_EXPORT_CPP void Allocate(unsigned nNewMinimumVarDataSize);

private:
	// this can only be created by CreateRecord in the RecordInfo
	Record() = default;

	void Init(int nFixedRecordSize, bool bContainsVarData);

public:
	E1ONLY_RECORDLIB_EXPORT_CPP int GetVarDataSize();

	E1ONLY_RECORDLIB_EXPORT_CPP void Reset(int nVarDataSize = 0);

	E1ONLY_RECORDLIB_EXPORT_CPP ~Record();

	// returns the offset within the var data to the start
	E1ONLY_RECORDLIB_EXPORT_CPP int AddVarData(const void* pVarData, unsigned nLen);

	E1ONLY_RECORDLIB_EXPORT_CPP void SetLength() const;

	E1ONLY_RECORDLIB_EXPORT_CPP RecordData* GetRecord();
	E1ONLY_RECORDLIB_EXPORT_CPP const RecordData* GetRecord() const;
};
}  // namespace SRC

#endif /* XSRCLIB_RECORDLIB_INCLUDE_RECORDLIB_RECORD_H */
