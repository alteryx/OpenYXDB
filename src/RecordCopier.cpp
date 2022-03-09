#include "stdafx.h"

#include "RecordLib/RecordCopier.h"

#include <algorithm>

#include "RecordLib/FieldTypes.h"
#include "RecordLib/RecordInfo.h"

#if !defined(SRCLIB_REPLACEMENT) && !defined(E2)
	#include "Base/Blob.h"
	#include "Base/crc.h"
	#include <Base/Glot.h>
#endif

namespace SRC {
struct RecordCopier::CopyCmd
{
	// if the field isn't changing its enough to know the positions
	int nSrcOffset;
	int nDestOffset;
	int nLen;

	// but if the data is changing, we need to do more...
	int nSrcFieldNum;
	int nDestFieldNum;

	bool bIsFieldChange;
	bool bIsVarData;
	unsigned nVarDataMaxBytes;

	inline bool operator<(const CopyCmd& o) const
	{
		return nSrcOffset < o.nSrcOffset;
	}
};

RecordCopier::RecordCopier(
	const RecordInfo& recInfoDest,
	const RecordInfo& recInfoSource,
	bool bSuppressSizeOnlyConvErrors,
	const char sep)
	: m_recInfoSource(recInfoSource)
	, m_recInfoDest(recInfoDest)
	, m_bSuppressSizeOnlyConvErrors(bSuppressSizeOnlyConvErrors)
	, m_DecimalSepChar(sep)
{
}

RecordCopier::~RecordCopier()
{
}

///////////////////////////////////////////////////////////////////////////////
//	class RecordCopier
void RecordCopier::Add(int nDestFieldNum, int nSourceFieldNum)
{
	m_vDeferredAdds.push_back(std::pair<int, int>(nDestFieldNum, nSourceFieldNum));
}

bool RecordCopier::IsValid() const
{
	return !m_vDeferredAdds.empty() || !m_vCopyCmds.empty();
}

void RecordCopier::DoAdd(int nDestFieldNum, int nSourceFieldNum)
{
	const FieldBase& fieldSource = *m_recInfoSource[nSourceFieldNum];
	const FieldBase& fieldDest = *m_recInfoDest[nDestFieldNum];

	CopyCmd copyCmd;

	copyCmd.nSrcFieldNum = nSourceFieldNum;
	copyCmd.nDestFieldNum = nDestFieldNum;

	copyCmd.bIsFieldChange = fieldSource.m_ft != fieldDest.m_ft || fieldSource.m_nRawSize != fieldDest.m_nRawSize
							 || fieldSource.m_nSize != fieldDest.m_nSize
							 || (fieldSource.m_ft == E_FT_FixedDecimal && fieldSource.m_nScale != fieldDest.m_nScale);

	if (!copyCmd.bIsFieldChange)
	{
		copyCmd.bIsFieldChange = false;
		copyCmd.nSrcOffset = fieldSource.GetOffset();
		copyCmd.nDestOffset = fieldDest.GetOffset();
		copyCmd.nLen = fieldSource.m_nRawSize;

		copyCmd.bIsVarData = fieldDest.m_bIsVarLength;
		// if the size is changing, we may need to truncate the data.
		if (copyCmd.bIsVarData)
			copyCmd.nVarDataMaxBytes = fieldDest.GetMaxBytes();
	}
	m_vCopyCmds.push_back(copyCmd);
}

void RecordCopier::DoneAdding()
{
	for (std::vector<std::pair<int, int>>::const_iterator it = m_vDeferredAdds.begin(); it != m_vDeferredAdds.end();
		 it++)
	{
		DoAdd(it->first, it->second);
	}
	m_vDeferredAdds.clear();

	// the goal here is to merge together memcpy cmds
	// it only works it the fields are consequtive and identical
	std::sort(m_vCopyCmds.begin(), m_vCopyCmds.end());

	int prevIndex = 0;
	for (unsigned x = 1; x < m_vCopyCmds.size(); ++x, ++prevIndex)
	{
		bool bCommandMerged = false;
		CopyCmd& thisCopyCmd = m_vCopyCmds[x];
		if (!thisCopyCmd.bIsVarData && !thisCopyCmd.bIsFieldChange)
		{
			CopyCmd& prevCopyCmd = m_vCopyCmds[prevIndex];
			if (!prevCopyCmd.bIsVarData && !prevCopyCmd.bIsFieldChange)
			{
				if (thisCopyCmd.nSrcOffset == (prevCopyCmd.nSrcOffset + prevCopyCmd.nLen)
					&& thisCopyCmd.nDestOffset == (prevCopyCmd.nDestOffset + prevCopyCmd.nLen))
				{
					prevCopyCmd.nLen += thisCopyCmd.nLen;
					bCommandMerged = true;
					prevIndex--;
				}
			}
		}
		if (!bCommandMerged && static_cast<unsigned>(prevIndex) != x - 1)
			m_vCopyCmds[prevIndex + 1] = thisCopyCmd;
	}
	m_vCopyCmds.resize(prevIndex + 1);
}

void RecordCopier::Copy(Record* pRecDest, const RecordData* pRecSrc) const
{
	if (m_vDeferredAdds.size() != 0)
	{
		assert(false);
		const_cast<RecordCopier*>(this)->DoneAdding();
	}
	for (std::vector<CopyCmd>::const_iterator it = m_vCopyCmds.begin(); it != m_vCopyCmds.end(); it++)
	{
		if (it->bIsFieldChange)
		{
			const FieldBase* pFieldDest = m_recInfoDest[it->nDestFieldNum];
			const FieldBase* pFieldSrc = m_recInfoSource[it->nSrcFieldNum];
			const GenericEngineBase* pSaveDestEngine = NULL;
			if (m_bSuppressSizeOnlyConvErrors && pFieldDest->m_ft == pFieldSrc->m_ft)
				pFieldDest->m_pGenericEngine = NULL;

			switch (pFieldDest->m_ft)
			{
				case E_FT_Bool:
					pFieldDest->SetFromBool(pRecDest, pFieldSrc->GetAsBool(pRecSrc));
					break;
				case E_FT_Byte:
				case E_FT_Int16:
				case E_FT_Int32:
					pFieldDest->SetFromInt32(pRecDest, pFieldSrc->GetAsInt32(pRecSrc));
					break;
				case E_FT_Int64:
					pFieldDest->SetFromInt64(pRecDest, pFieldSrc->GetAsInt64(pRecSrc));
					break;
				case E_FT_FixedDecimal:
					switch (pFieldSrc->m_ft)
					{
						case E_FT_Byte:
						case E_FT_Int16:
						case E_FT_Int32:
							pFieldDest->SetFromInt32(pRecDest, pFieldSrc->GetAsInt32(pRecSrc));
							break;
						case E_FT_Int64:
							pFieldDest->SetFromInt64(pRecDest, pFieldSrc->GetAsInt64(pRecSrc));
							break;
						case E_FT_Float:
						case E_FT_Double:
							pFieldDest->SetFromDouble(pRecDest, pFieldSrc->GetAsDouble(pRecSrc));
							break;
						case E_FT_WString:
						case E_FT_V_WString:
						case E_FT_String:
						case E_FT_V_String:
							if (CommaDecimalEnabled())  // user has enabled commas as decimal separators
							{
								TFieldVal<AStringVal> tSrcVal = pFieldSrc->GetAsAString(pRecSrc);

								if (!tSrcVal.bIsNull)
								{
									pFieldDest->SetFromString(pRecDest, ConvertDecimalSep(tSrcVal.value));
									break;
								}
							}
							// fall through for default handling...
						default:
							pFieldDest->SetFromString(pRecDest, pFieldSrc->GetAsAString(pRecSrc));
							break;
					}
					break;
				case E_FT_Float:
				case E_FT_Double:
					if (CommaDecimalEnabled()          // user has enabled commas as decimal separators
						&& IsString(pFieldSrc->m_ft))  // and we are converting from a string to a float/double
					{
						TFieldVal<AStringVal> tSrcVal = pFieldSrc->GetAsAString(pRecSrc);
						TFieldVal<TBlobVal<char>> val(tSrcVal.bIsNull, tSrcVal.value);
						pFieldDest->SetFromDouble(
							pRecDest, ConvertStringToDoubleWithConversionErrors(val, pFieldSrc, GetDecimalSepChar()));
					}
					else
					{
						pFieldDest->SetFromDouble(pRecDest, pFieldSrc->GetAsDouble(pRecSrc));
					}
					break;
				case E_FT_WString:
				case E_FT_V_WString:
					if (CommaDecimalEnabled()  // user has enabled commas as decimal separators
						&& (pFieldSrc->m_ft == E_FT_Double || pFieldSrc->m_ft == E_FT_FixedDecimal
							|| pFieldSrc->m_ft == E_FT_Float))
					{
						// we can get as AString here since we know we're only converting from number types
						TFieldVal<AStringVal> tSrcVal = pFieldSrc->GetAsAString(pRecSrc);
						if (!tSrcVal.bIsNull)
						{
							pFieldDest->SetFromString(pRecDest, ConvertDecimalSep(tSrcVal.value));
							break;
						}
					}
					pFieldDest->SetFromString(pRecDest, pFieldSrc->GetAsWString(pRecSrc));
					break;
				case E_FT_String:
				case E_FT_V_String:
					if (CommaDecimalEnabled()  // user has enabled commas as decimal separators
						&& (pFieldSrc->m_ft == E_FT_Double || pFieldSrc->m_ft == E_FT_FixedDecimal
							|| pFieldSrc->m_ft == E_FT_Float))
					{
						TFieldVal<AStringVal> tSrcVal = pFieldSrc->GetAsAString(pRecSrc);
						if (!tSrcVal.bIsNull)
						{
							pFieldDest->SetFromString(pRecDest, ConvertDecimalSep(tSrcVal.value));
							break;
						}
					}
					// fall through for default handling...
				case E_FT_Date:
				case E_FT_Time:
				case E_FT_DateTime:
					pFieldDest->SetFromString(pRecDest, pFieldSrc->GetAsAString(pRecSrc));
					break;
				case E_FT_Blob:
					pFieldDest->SetFromBlob(pRecDest, pFieldSrc->GetAsBlob(pRecSrc));
					break;
				case E_FT_SpatialObj:
					pFieldDest->SetFromSpatialBlob(pRecDest, pFieldSrc->GetAsSpatialBlob(pRecSrc));
					break;
				case E_FT_Unknown:
					break;
			}
			if (m_bSuppressSizeOnlyConvErrors && pFieldDest->m_ft == pFieldSrc->m_ft)
				pFieldDest->m_pGenericEngine = pSaveDestEngine;
		}
		else if (it->bIsVarData)
		{
			const FieldBase* pFieldDest = m_recInfoDest[it->nDestFieldNum];
			const FieldBase* pFieldSrc = m_recInfoSource[it->nSrcFieldNum];
			BlobVal val = m_recInfoSource.GetVarDataValue(pRecSrc, pFieldSrc->GetOffset());

			// truncate the data if need be.
			unsigned nNewLen = unsigned(std::min(it->nVarDataMaxBytes, val.nLength));
			m_recInfoDest.SetVarDataValue(pRecDest, pFieldDest->GetOffset(), nNewLen, val.pValue);
		}
		else
		{
			// we can just copy a block of raw data over
			memcpy(
				static_cast<char*>(pRecDest->m_pRecord) + it->nDestOffset, ToCharP(pRecSrc) + it->nSrcOffset, it->nLen);
		}
	}
}

void RecordCopier::SetDestToNull(Record* pRecDest) const
{
	for (unsigned x = 0; x < m_recInfoDest.NumFields(); ++x)
		m_recInfoDest[x]->SetNull(pRecDest);
}

const AString& RecordCopier::ConvertDecimalSep(const AStringVal& tSrcVal) const
{
	m_strATemp.Assign(tSrcVal.pValue, tSrcVal.nLength);
	for (auto* ptr = m_strATemp.Lock(); *ptr; ++ptr)
	{
		auto& c = *ptr;
		if (c == '.')
			c = ',';
		else if (c == ',')
			c = '.';
	}
	m_strATemp.Unlock(tSrcVal.nLength);
	return m_strATemp;
}

bool RecordCopier::CommaDecimalEnabled() const
{
	return m_DecimalSepChar == ',';
}

const char RecordCopier::GetDecimalSepChar() const
{
	return m_DecimalSepChar;
}

}  // namespace SRC
