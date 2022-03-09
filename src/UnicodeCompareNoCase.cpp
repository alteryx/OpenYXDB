#include "stdafx.h"

#include "UnicodeCompareNoCase.h"

#include "Base/EncodingTraits/UTF16Traits.h"
#include "Base/UnicodeCaseFoldingIterator.h"

namespace SRC {

namespace {

template <class EncodingTraits, class TChar>
int TCompareNoCase(const TChar* pA, const TChar* pB)
{
	if (pA == nullptr)
		return pB == nullptr ? 0 : -1;
	else if (pB == nullptr)
		return 1;

	CaseFoldingIterator<EncodingTraits, TChar> itA = pA;
	CaseFoldingIterator<EncodingTraits, TChar> itB = pB;

	while (!itA.AtEnd() && !itB.AtEnd() && itA == itB)
	{
		++itA;
		++itB;
	}

	return itA - itB;
}

template <class EncodingTraits, class TChar>
int TCompareNoCaseN(const TChar* pA, const TChar* pB, int N)
{
	if (N == 0)
		return 0;
	if (pA == nullptr)
		return pB == nullptr ? 0 : -1;
	else if (pB == nullptr)
		return 1;

	CaseFoldingIterator<EncodingTraits, TChar> itA = pA;
	CaseFoldingIterator<EncodingTraits, TChar> itB = pB;

	for (;;)
	{
		if (!(itA == itB))
			return itA - itB;  // return a difference
		++itA;
		++itB;
		// because case folding can increase the number of code points, we have
		// the odd case where we reach N units in one input before the other!
		// For example, case blind "ß" == "ss"
		// I am going to judge by the B input, since we normally have A being
		// the long string, and B be the short one we're matching into.
		if (itB.AtEnd(N))
			return 0;  // they matched for N units of B.
		// a classic strnicmp can return the difference between characters when
		// the increment has reached the end. Here incrementing to the end has
		// changed the value of the "last" character, so we have nothing to
		// compare. These strings may not be null terminated, so we can't compare
		// one's ending null to the other one's character.
		if (itA.AtEnd())
		{
			return -1;  // B is longer than A
		}
	}
}

}  // namespace

int CompareNoCaseUnicode::CompareNoCaseUTF16(const U16unit* pA, const U16unit* pB)
{
	return TCompareNoCase<UTF16Traits>(pA, pB);
}
int CompareNoCaseUnicode::CompareNoCaseUTF16(const U16unit* pA, const U16unit* pB, int N)
{
	return TCompareNoCaseN<UTF16Traits>(pA, pB, N);
}

}  // namespace SRC
