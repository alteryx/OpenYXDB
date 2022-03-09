// Copyright(C) 2005 - 2019 Alteryx, Inc. All rights reserved.
// This file is distributed to Alteryx customers as part of the
// Software Development Kit.

#pragma once

namespace SRC {
template <class ValType>
struct TFieldVal
{
	bool bIsNull;
	ValType value;

	inline TFieldVal()
		: bIsNull(false)
	{
	}

	inline TFieldVal(bool _bIsNull, ValType _value)
		: bIsNull(_bIsNull)
		, value(_value)
	{
	}
};
}  // namespace SRC
