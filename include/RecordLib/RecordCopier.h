// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

#pragma once

#include <vector>

#include "FieldBase.h"
#include "RecordLibExport.h"

namespace SRC {
class Record;
class RecordInfo;

///////////////////////////////////////////////////////////////////////////////
//	class RecordCopier
class RecordCopier : public SmartPointerRefObj_Base
{
	const RecordInfo& m_recInfoSource;
	const RecordInfo& m_recInfoDest;
	bool m_bSuppressSizeOnlyConvErrors;

	std::vector<std::pair<int, int>> m_vDeferredAdds;
	void DoAdd(int nDestFieldNum, int nSourceFieldNum);
	bool CommaDecimalEnabled() const;
	const char GetDecimalSepChar() const;

	struct CopyCmd;
	std::vector<CopyCmd> m_vCopyCmds;

	mutable AString m_strATemp;
	const char m_DecimalSepChar;

public:
	// recInfoDest does not need to be complete until DoneAdding is called.  All adds are deferred until then
	E1ONLY_RECORDLIB_EXPORT_CPP RecordCopier(
		const RecordInfo& recInfoDest,
		const RecordInfo& recInfoSource,
		bool bSuppressSizeOnlyConvErrors = false,
		const char sep = '.');
	E1ONLY_RECORDLIB_EXPORT_CPP ~RecordCopier();

	RecordCopier(const RecordCopier&) = delete;
	RecordCopier& operator=(const RecordCopier&) = delete;

	E1ONLY_RECORDLIB_EXPORT_CPP void Add(int nDestFieldNum, int nSourceFieldNum);
	E1ONLY_RECORDLIB_EXPORT_CPP bool IsValid() const;

	// recInfoDest MUST be complete before calling DoneAdding
	E1ONLY_RECORDLIB_EXPORT_CPP void DoneAdding();

	E1ONLY_RECORDLIB_EXPORT_CPP void Copy(Record* pRecDest, const RecordData* pRecSrc) const;

	RECORDLIB_EXPORT_CPP void SetDestToNull(Record* pRecDest) const;

	E1ONLY_RECORDLIB_EXPORT_CPP const AString& ConvertDecimalSep(const AStringVal& tSrcVal) const;
};
}  // namespace SRC
