// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

#pragma once

#ifndef SRCLIB_REPLACEMENT
	#include "Base/SRC_string.h"
#endif

namespace SRC {
// Similar to gsl::span and string_view, this encapsulates a pointer (to memory we do not own)
// and a length. Alteryx made this before those others existed, and is used all over the place.

template <class ValType>
struct TBlobVal
{
	unsigned nLength;
	const ValType* pValue;
	inline TBlobVal()
		: nLength(0)
		, pValue(NULL)
	{
	}
	inline TBlobVal(unsigned _nLength, const ValType* _pValue)
		: nLength(_nLength)
		, pValue(_pValue)
	{
	}
};

template <>
inline TBlobVal<char>::TBlobVal()
	: nLength(0)
	, pValue("")
{
}
template <>
inline TBlobVal<U16unit>::TBlobVal()
	: nLength(0)
	, pValue(U16(""))
{
}

// "WStringVal" because it takes the same unit as a WString in e1, but in e2 it takes the same unit
// as a UString. It has nothing else to do with WString
struct WStringVal : public TBlobVal<String::TChar>
{
	inline WStringVal()
	{
	}
	inline WStringVal(unsigned _nLength, const String::TChar* _pValue)
		: TBlobVal<String::TChar>(_nLength, _pValue)
	{
	}
};

struct AStringVal : public TBlobVal<char>
{
	inline AStringVal()
	{
	}
	inline AStringVal(unsigned _nLength, const char* _pValue)
		: TBlobVal<char>(_nLength, _pValue)
	{
	}
};

typedef TBlobVal<void> BlobVal;
}  // namespace SRC
