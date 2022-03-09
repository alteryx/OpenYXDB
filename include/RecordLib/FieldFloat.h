// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

#pragma once

#include <cmath>

#include <float.h>

#include "FieldNum.h"

namespace SRC {
template <int ft, class T_Num>
class RECORDLIB_EXPORT_CPP T_Field_Float : public Field_Num<ft, T_Num>
{
	template <class TChar>
	inline void TSetFromString(Record* pRecord, const TChar* pVal, size_t nLen) const
	{
		if (nLen == 0)
		{
			this->SetNull(pRecord);
		}
		else
		{
			double dVal;
			unsigned nNumCharsUsed = ConvertToDouble(pVal, dVal);
			if (0 == nNumCharsUsed || std::isnan(dVal))
			{
				if (this->IsReportingFieldConversionErrors())
					this->ReportFieldConversionError(XMSG("@1 is not a valid number.", ConvertToWString(pVal)));
				this->SetNull(pRecord);
			}
			else
			{
				if (this->IsReportingFieldConversionErrors() && nNumCharsUsed != nLen && this->m_pGenericEngine)
				{
					if (pVal[nNumCharsUsed] == ',')
						this->ReportFieldConversionError(
							XMSG("@1 stopped converting at a comma. It might be invalid.", ConvertToWString(pVal)));
					else
						this->ReportFieldConversionError(
							XMSG("@1 lost information in translation", ConvertToWString(pVal)));
				}

				this->SetVal(pRecord, T_Num(dVal));
			}
		}
	}

public:
	inline T_Field_Float(String strFieldName, int nScale = -1)
		: Field_Num<ft, T_Num>(strFieldName, nScale)
	{
	}

	inline T_Field_Float(const FieldSchema& fieldSchema)
		: Field_Num<ft, T_Num>(fieldSchema)
	{
	}

	virtual SmartPointerRefObj<FieldBase> Copy() const
	{
		return this->CopyHelper(new T_Field_Float<ft, T_Num>(this->GetFieldName(), this->m_nScale));
	}

	virtual void SetFromString(Record* pRecord, const char* pVal, size_t nLen) const
	{
		ForceNullTerminated<char>(this->m_astrTemp, pVal, nLen);
		return TSetFromString(pRecord, pVal, nLen);
	}

	virtual void SetFromString(Record* pRecord, const U16unit* pVal, size_t nLen) const
	{
		WString strTemp;
		ForceNullTerminated<U16unit>(strTemp, pVal, nLen);
		return TSetFromString(pRecord, pVal, nLen);
	}
};

typedef T_Field_Float<E_FT_Float, float> Field_Float;
typedef T_Field_Float<E_FT_Double, double> Field_Double;
}  // namespace SRC
