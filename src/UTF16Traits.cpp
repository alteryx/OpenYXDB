#include "stdafx.h"

#include "Base/EncodingTraits/UTF16Traits.h"

#ifndef SRCLIB_REPLACEMENT
	#include "Base/Src_Error.h"
#else
	#include "SrcLib_Replacement.h"
#endif
#include "Base/Unicode.h"

namespace SRC {
using namespace SRC::Unicode;

namespace {
static constexpr char32_t LEAD_OFFSET = LEAD_SURROGATE_MIN - (0x10000 >> 10);
static constexpr char32_t SURROGATE_OFFSET = 0x10000u - (LEAD_SURROGATE_MIN << 10) - TRAIL_SURROGATE_MIN;
static constexpr char32_t REPLACEMENT_CHAR = 0xFFFD;  // the unicode "replacement char"

template <class TChar>
unsigned TWriteCodePoint(const char32_t cp, TChar*& buffer)
{
	if (cp > 0xffff)  // We need a surrogate pair
	{
		*buffer++ = static_cast<TChar>((cp >> 10) + LEAD_OFFSET);
		*buffer++ = static_cast<TChar>((cp & 0x3ff) + TRAIL_SURROGATE_MIN);
		return 2;
	}
	else
	{
		*buffer++ = static_cast<TChar>(cp);
		return 1;
	}
}
}  // namespace

STRING_EXPORT unsigned /*static*/ UTF16Traits::WriteCodePoint(const char32_t cp, U16unit*& buffer)
{
	return TWriteCodePoint(cp, buffer);
}

STRING_EXPORT char32_t UTF16Traits::CodePoint::Number() const noexcept
{
	char32_t cp = m_buffer[0];
	if (IsUTF16LeadSurrogate(cp))
	{
		if (m_length == 1 || m_buffer[1] == 0)
			return '?';  // "Invalid UTF-16 data: lead surrogate without trailer" - treat it as ?

		U16unit trailSurrogate(static_cast<U16unit>(m_buffer[1] & 0xffff));
		if (IsUTF16TrailSurrogate(trailSurrogate))
			cp = (cp << 10) + trailSurrogate + SURROGATE_OFFSET;
		else
			return '?';  // "Invalid UTF-16 data: unit after lead surrogate is not a trailing surrogate" - treat it as ?
	}
	else if (IsUTF16TrailSurrogate(cp))
		return '?';  // "Invalid UTF-16 data: trailing surrogate that doesn't follow a leading surrogate" - treat it as ?
	return cp;
}

UTF16Traits::CodePointIterator::CodePointIterator(const U16unit* str, unsigned pos)
	: m_str(str)
	, m_Pos(pos)
{
}

UTF16Traits::CodePointIterator& UTF16Traits::CodePointIterator::operator++()
{
	// It is ok to iterate past a nul (it is up to the caller to check).
	// But don't iterate past a nul from a corrupt utf16 lead with a missing trail character.
	auto incrementAmt = unsigned{ 1 };
	if (IsUTF16LeadSurrogate(m_str[m_Pos]) && m_str[m_Pos + 1] != 0)
	{
		++incrementAmt;
	}
	m_Pos += incrementAmt;
	return *this;
}

UTF16Traits::CodePointIterator& UTF16Traits::CodePointIterator::operator--()
{
	while (m_Pos > 0 && IsUTF16TrailSurrogate(m_str[--m_Pos]))
		;
	return *this;
}

const UTF16Traits::CodePoint UTF16Traits::CodePointIterator::operator*() const
{
	CodePoint ret(&m_str[m_Pos], IsUTF16LeadSurrogate(m_str[m_Pos]) ? 2 : 1);
	return ret;
}

UTF16Traits::CodePointIterator& UTF16Traits::CodePointIterator::operator+=(unsigned n)
{
	for (auto i = unsigned{ 0 }; i < n; ++i)
	{
		this->operator++();
	}
	return *this;
}

UTF16Traits::CodePointIterator& UTF16Traits::CodePointIterator::operator-=(unsigned n)
{
	for (; n > 0 && m_Pos > 0; --n)
	{
		auto ch = m_str[--m_Pos];
		if (IsUTF16TrailSurrogate(ch))
		{
			if (m_Pos <= 0)
				throw Error(XMSG("Invalid UTF-16 trailing surrogate"));
			// is it worth the effort to validate that the previous character is a lead surrogate?
			--m_Pos;
		}
	}
	return *this;
}

unsigned UTF16Traits::CodePointIterator::LengthPoints() const
{
	unsigned pos = m_Pos;
	for (unsigned res = 0;; ++res)
	{
		auto ch = m_str[pos];
		if (ch == 0)
			return res;
		if (IsUTF16LeadSurrogate(ch))
			pos += 2;
		else
			++pos;
	}
}

int UTF16Traits::CodePointIterator::ValidNumber() const noexcept
{
	// ensure int can support all of the available unicode characters
	static_assert(TypeCanHoldUnicodeAssert<int>(), "int cannot contain all possible unicode");

	auto chPtr = &m_str[m_Pos];
	if (IsUTF16TrailSurrogate(*chPtr))
	{
		// character starts as a trail surrogate, error!
		return -1;
	}

	if (IsUTF16LeadSurrogate(*chPtr) && !IsUTF16TrailSurrogate(*(chPtr + 1)))
	{
		// character is a lead surrogate, the next character isn't a trail surrogate, error!
		return -1;
	}

	auto codepoint = CodePoint(chPtr, IsUTF16LeadSurrogate(*chPtr) ? 2 : 1);
	auto unicode = codepoint.Number();
	if (IsValidUnicode(unicode))
	{
		return static_cast<int>(unicode);
	}
	return -1;
}

// give the value of the code point at the current position, and advance to
// the next code point.
// Does its best to repair broken UTF-16 (Lead surrogate without trail, or
// pointing at a trailing) by returning the replacement character (U+FFFD)
// and advancing one char16.
char32_t UTF16Traits::CodePointIterator::NumberAdvRepair() noexcept
{
	char32_t ret = m_str[m_Pos];
	if (ret < LEAD_SURROGATE_MIN || TRAIL_SURROGATE_MAX < ret)
	{
		++m_Pos;  // probably almost always here, all good BMP chars
		return ret;
	}
	if (LEAD_SURROGATE_MAX < ret || !IsUTF16TrailSurrogate(m_str[m_Pos + 1]))
	{
		++m_Pos;  // should we throw when we turn bad data into Replacement?
		return REPLACEMENT_CHAR;
	}
	// a 2-char16 sequence.
	ret = (ret << 10) + m_str[m_Pos + 1] + SURROGATE_OFFSET;
	m_Pos += 2;
	return ret;
}

// tell how many code points there are from here to where RHS points
//STRING_EXPORT unsigned UTF16Traits::CodePointIterator::operator-(const CodePointIterator& rhs) const
//{
//	unsigned nUnits=static_cast<unsigned>((rhs.m_str + rhs.m_Pos) - m_str);
//	unsigned pos = m_Pos;
//	for (unsigned res = 0; ; ++res)
//	{
//		if (pos >= nUnits)
//			return res;
//		auto ch = m_str[pos];
//		if (ch == 0)
//			return res;
//		if (IsUTF16LeadSurrogate(ch))
//			pos += 2;
//		else
//			++pos;
//	}
//}

UTF16Traits::CodePoints::CodePoints(const U16unit* p, int nLength)
	: m_Begin(p, 0)
	, m_End(p, nLength)
{
}
}  // namespace SRC
